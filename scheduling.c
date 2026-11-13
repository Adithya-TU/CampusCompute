#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduling.h"

static int read_int(const char *prompt, int min, int max) {
    char line[128], extra;
    int value;
    for (;;) {
        printf("%s", prompt);
        if (!fgets(line, sizeof(line), stdin)) return min;
        if (sscanf(line, " %d %c", &value, &extra) == 1 && value >= min && value <= max)
            return value;
        printf("Invalid input. Enter an integer from %d to %d.\n", min, max);
    }
}

static void print_results(const char *name, Job jobs[], int n, const int order[], int count) {
    double total_wait = 0, total_turn = 0;
    printf("\n%s execution order: ", name);
    for (int i = 0; i < count; i++)
        printf("J%d%s", order[i], i == count - 1 ? "" : " -> ");
    printf("\n\nJob\tArrival\tBurst\tPriority\tCompletion\tTurnaround\tWaiting\n");
    for (int i = 0; i < n; i++) {
        jobs[i].turnaround = jobs[i].completion - jobs[i].arrival;
        jobs[i].waiting = jobs[i].turnaround - jobs[i].burst;
        total_wait += jobs[i].waiting;
        total_turn += jobs[i].turnaround;
        printf("J%d\t%d\t%d\t%d\t\t%d\t\t%d\t\t%d\n",
               jobs[i].id, jobs[i].arrival, jobs[i].burst, jobs[i].priority,
               jobs[i].completion, jobs[i].turnaround, jobs[i].waiting);
    }
    printf("Average waiting time: %.2f\n", total_wait / n);
    printf("Average turnaround time: %.2f\n", total_turn / n);
}

static void run_fcfs(Job input[], int n) {
    Job jobs[MAX_JOBS];
    int order[MAX_JOBS], count = 0, time = 0;
    memcpy(jobs, input, n * sizeof(Job));
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (jobs[j].arrival < jobs[i].arrival ||
                (jobs[j].arrival == jobs[i].arrival && jobs[j].id < jobs[i].id)) {
                Job t = jobs[i]; jobs[i] = jobs[j]; jobs[j] = t;
            }
    for (int i = 0; i < n; i++) {
        if (time < jobs[i].arrival) time = jobs[i].arrival;
        jobs[i].start = time;
        time += jobs[i].burst;
        jobs[i].completion = time;
        order[count++] = jobs[i].id;
    }
    print_results("FCFS", jobs, n, order, count);
}

static void run_priority(Job input[], int n) {
    Job jobs[MAX_JOBS];
    int done[MAX_JOBS] = {0}, order[MAX_JOBS], count = 0, time = 0;
    memcpy(jobs, input, n * sizeof(Job));
    while (count < n) {
        int best = -1;
        for (int i = 0; i < n; i++) {
            if (!done[i] && jobs[i].arrival <= time &&
                (best == -1 || jobs[i].priority < jobs[best].priority ||
                 (jobs[i].priority == jobs[best].priority && jobs[i].arrival < jobs[best].arrival) ||
                 (jobs[i].priority == jobs[best].priority && jobs[i].arrival == jobs[best].arrival && jobs[i].id < jobs[best].id)))
                best = i;
        }
        if (best == -1) {
            time++;
            continue;
        }
        jobs[best].start = time;
        time += jobs[best].burst;
        jobs[best].completion = time;
        done[best] = 1;
        order[count++] = jobs[best].id;
    }
    print_results("Non-preemptive Priority (lower number = higher priority)", jobs, n, order, count);
}

static void run_rr(Job input[], int n, int quantum) {
    Job jobs[MAX_JOBS];
    int order[2000], count = 0, finished = 0, time = 0;
    memcpy(jobs, input, n * sizeof(Job));
    for (int i = 0; i < n; i++) jobs[i].remaining = jobs[i].burst;
    while (finished < n) {
        int progressed = 0;
        for (int i = 0; i < n; i++) {
            if (jobs[i].remaining > 0 && jobs[i].arrival <= time) {
                progressed = 1;
                int slice = jobs[i].remaining < quantum ? jobs[i].remaining : quantum;
                time += slice;
                jobs[i].remaining -= slice;
                if (count < 2000) order[count++] = jobs[i].id;
                if (jobs[i].remaining == 0) {
                    jobs[i].completion = time;
                    finished++;
                }
            }
        }
        if (!progressed) time++;
    }
    print_results("Round Robin", jobs, n, order, count);
}

void run_scheduling(void) {
    Job jobs[MAX_JOBS] = {0};
    char line[128], extra;
    int n, quantum;
    printf("\n--- Job Management and CPU Scheduling ---\n");
    for (;;) {
        printf("Number of jobs (1-%d): ", MAX_JOBS);
        if (!fgets(line, sizeof(line), stdin)) return;
        if (sscanf(line, " %d %c", &n, &extra) == 1 && n >= 1 && n <= MAX_JOBS) break;
        printf("Invalid number of jobs.\n");
    }
    for (int i = 0; i < n; i++) {
        jobs[i].id = i + 1;
        char prompt[100];
        snprintf(prompt, sizeof(prompt), "Job J%d arrival time (0-100000): ", i + 1);
        jobs[i].arrival = read_int(prompt, 0, 100000);
        snprintf(prompt, sizeof(prompt), "Job J%d CPU burst (1-100000): ", i + 1);
        jobs[i].burst = read_int(prompt, 1, 100000);
        snprintf(prompt, sizeof(prompt), "Job J%d priority (1-100, lower is higher): ", i + 1);
        jobs[i].priority = read_int(prompt, 1, 100);
    }
    run_fcfs(jobs, n);
    run_priority(jobs, n);
    quantum = read_int("\nRound Robin time quantum (1-100000): ", 1, 100000);
    run_rr(jobs, n, quantum);
    printf("\nTie-breaking: earlier arrival, then lower job ID. Priority: lower number is higher priority.\n");
}
