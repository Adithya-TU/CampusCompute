#ifndef SCHEDULING_H
#define SCHEDULING_H

#define MAX_JOBS 20

typedef struct {
    int id;
    int arrival;
    int burst;
    int priority;
    int remaining;
    int start;
    int completion;
    int waiting;
    int turnaround;
} Job;

void run_scheduling(void);

#endif
