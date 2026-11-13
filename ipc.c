#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <errno.h>
#include "ipc.h"

#define SHM_NAME "/campuscompute_shm"
#define SEM_NAME "/campuscompute_sem"

typedef struct {
    int value;
    char message[128];
} SharedData;

void run_ipc(void) {
    int fd = -1, created = 0;
    sem_t *sem = SEM_FAILED;
    SharedData *data = MAP_FAILED;
    pid_t child;

    fd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd >= 0) created = 1;
    else if (errno == EEXIST) fd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (fd < 0) { perror("shm_open"); return; }
    if (ftruncate(fd, sizeof(SharedData)) == -1) { perror("ftruncate"); close(fd); if(created) shm_unlink(SHM_NAME); return; }

    data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) { perror("mmap"); close(fd); if(created) shm_unlink(SHM_NAME); return; }

    sem = sem_open(SEM_NAME, O_CREAT, 0600, 0);
    if (sem == SEM_FAILED) { perror("sem_open"); munmap(data, sizeof(SharedData)); close(fd); if(created) shm_unlink(SHM_NAME); return; }

    child = fork();
    if (child < 0) {
        perror("fork");
    } else if (child == 0) {
        if (sem_wait(sem) == -1) { perror("sem_wait"); _exit(1); }
        printf("Child read shared memory: value=%d, message=\"%s\"\n", data->value, data->message);
        fflush(stdout);
        _exit(0);
    } else {
        data->value = 242;
        snprintf(data->message, sizeof(data->message), "Hello from CampusCompute parent process");
        printf("Parent wrote shared memory and signals child.\n");
        if (sem_post(sem) == -1) perror("sem_post");
        int status;
        if (waitpid(child, &status, 0) == -1) perror("waitpid");
        else if (WIFEXITED(status)) printf("Child process exited with status %d.\n", WEXITSTATUS(status));
    }

    sem_close(sem);
    sem_unlink(SEM_NAME);
    munmap(data, sizeof(SharedData));
    close(fd);
    shm_unlink(SHM_NAME);
    printf("IPC cleanup complete: shared memory and named semaphore unlinked.\n");
}
