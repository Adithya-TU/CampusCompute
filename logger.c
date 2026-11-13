#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "logger.h"

#define QUEUE_SIZE 5
#define MESSAGE_SIZE 160

typedef struct {
    char items[QUEUE_SIZE][MESSAGE_SIZE];
    int in, out, count, producers_done;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty, not_full;
} LogQueue;

static LogQueue queue_data;
static FILE *log_file;

static void enqueue(const char *message) {
    pthread_mutex_lock(&queue_data.mutex);
    while (queue_data.count == QUEUE_SIZE)
        pthread_cond_wait(&queue_data.not_full, &queue_data.mutex);
    snprintf(queue_data.items[queue_data.in], MESSAGE_SIZE, "%s", message);
    queue_data.in = (queue_data.in + 1) % QUEUE_SIZE;
    queue_data.count++;
    pthread_cond_signal(&queue_data.not_empty);
    pthread_mutex_unlock(&queue_data.mutex);
}

static void *producer(void *arg) {
    long id = (long)arg;
    for (int i = 1; i <= 5; i++) {
        char message[MESSAGE_SIZE];
        snprintf(message, sizeof(message), "Producer %ld generated event %d", id, i);
        enqueue(message);
    }
    pthread_mutex_lock(&queue_data.mutex);
    queue_data.producers_done++;
    pthread_cond_signal(&queue_data.not_empty);
    pthread_mutex_unlock(&queue_data.mutex);
    return NULL;
}

static void *consumer(void *arg) {
    (void)arg;
    for (;;) {
        pthread_mutex_lock(&queue_data.mutex);
        while (queue_data.count == 0 && queue_data.producers_done < 2)
            pthread_cond_wait(&queue_data.not_empty, &queue_data.mutex);
        if (queue_data.count == 0 && queue_data.producers_done == 2) {
            pthread_mutex_unlock(&queue_data.mutex);
            break;
        }
        char message[MESSAGE_SIZE];
        snprintf(message, sizeof(message), "%s", queue_data.items[queue_data.out]);
        queue_data.out = (queue_data.out + 1) % QUEUE_SIZE;
        queue_data.count--;
        pthread_cond_signal(&queue_data.not_full);
        pthread_mutex_unlock(&queue_data.mutex);

        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char stamp[32];
        strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(log_file, "[%s] Consumer processed: %s\n", stamp, message);
        printf("[%s] Consumer processed: %s\n", stamp, message);
        fflush(log_file);
    }
    return NULL;
}

void run_logger(void) {
    pthread_t p1, p2, c1;
    memset(&queue_data, 0, sizeof(queue_data));
    if (pthread_mutex_init(&queue_data.mutex, NULL) != 0 ||
        pthread_cond_init(&queue_data.not_empty, NULL) != 0 ||
        pthread_cond_init(&queue_data.not_full, NULL) != 0) {
        fprintf(stderr, "Could not initialize logger synchronization objects.\n");
        return;
    }
    log_file = fopen("logs/campuscompute.log", "a");
    if (!log_file) {
        perror("logs/campuscompute.log");
        pthread_mutex_destroy(&queue_data.mutex);
        pthread_cond_destroy(&queue_data.not_empty);
        pthread_cond_destroy(&queue_data.not_full);
        return;
    }
    if (pthread_create(&p1, NULL, producer, (void *)1L) != 0 ||
        pthread_create(&p2, NULL, producer, (void *)2L) != 0 ||
        pthread_create(&c1, NULL, consumer, NULL) != 0) {
        fprintf(stderr, "Could not create logger threads.\n");
        fclose(log_file);
        return;
    }
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_join(c1, NULL);
    fclose(log_file);
    pthread_mutex_destroy(&queue_data.mutex);
    pthread_cond_destroy(&queue_data.not_empty);
    pthread_cond_destroy(&queue_data.not_full);
    printf("Logger finished. Messages appended to logs/campuscompute.log. All threads joined.\n");
}
