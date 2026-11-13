#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banker.h"

#define MAX_P 10
#define MAX_R 10

static int get_value(const char *prompt, int min, int max) {
    char line[128], extra;
    int x;
    for (;;) {
        printf("%s", prompt);
        if (!fgets(line, sizeof(line), stdin)) return min;
        if (sscanf(line, " %d %c", &x, &extra) == 1 && x >= min && x <= max) return x;
        printf("Enter a valid integer from %d to %d.\n", min, max);
    }
}

static void check_safety(int p, int r, int available[], int allocation[MAX_P][MAX_R],
                         int maximum[MAX_P][MAX_R]) {
    int need[MAX_P][MAX_R], work[MAX_R], finish[MAX_P] = {0}, sequence[MAX_P], count = 0;
    for (int i = 0; i < p; i++)
        for (int j = 0; j < r; j++) {
            need[i][j] = maximum[i][j] - allocation[i][j];
            if (need[i][j] < 0) {
                printf("Invalid state: allocation exceeds maximum claim.\n");
                return;
            }
        }
    memcpy(work, available, r * sizeof(int));
    printf("\nNeed matrix:\n");
    for (int i = 0; i < p; i++) {
        printf("P%d: ", i);
        for (int j = 0; j < r; j++) printf("%d ", need[i][j]);
        printf("\n");
    }
    while (count < p) {
        int found = 0;
        for (int i = 0; i < p; i++) {
            if (!finish[i]) {
                int possible = 1;
                for (int j = 0; j < r; j++)
                    if (need[i][j] > work[j]) possible = 0;
                if (possible) {
                    for (int j = 0; j < r; j++) work[j] += allocation[i][j];
                    finish[i] = 1;
                    sequence[count++] = i;
                    found = 1;
                }
            }
        }
        if (!found) break;
    }
    if (count == p) {
        printf("SAFE state. Safe sequence: ");
        for (int i = 0; i < p; i++) printf("P%d%s", sequence[i], i == p - 1 ? "\n" : " -> ");
    } else {
        printf("UNSAFE state. No safe sequence exists for the supplied state.\n");
        printf("Processes that could not finish: ");
        for (int i = 0; i < p; i++) if (!finish[i]) printf("P%d ", i);
        printf("\n");
    }
}

void run_banker(void) {
    int p = get_value("Number of processes (1-10): ", 1, MAX_P);
    int r = get_value("Number of resource types (1-10): ", 1, MAX_R);
    int available[MAX_R], allocation[MAX_P][MAX_R] = {{0}}, maximum[MAX_P][MAX_R] = {{0}};
    printf("Enter Available vector (%d values):\n", r);
    for (int j = 0; j < r; j++) {
        char prompt[80]; snprintf(prompt, sizeof(prompt), "Available R%d: ", j);
        available[j] = get_value(prompt, 0, 100000);
    }
    printf("Enter Allocation matrix:\n");
    for (int i = 0; i < p; i++) for (int j = 0; j < r; j++) {
        char prompt[80]; snprintf(prompt, sizeof(prompt), "Allocation P%d R%d: ", i, j);
        allocation[i][j] = get_value(prompt, 0, 100000);
    }
    printf("Enter Maximum matrix:\n");
    for (int i = 0; i < p; i++) for (int j = 0; j < r; j++) {
        char prompt[80]; snprintf(prompt, sizeof(prompt), "Maximum P%d R%d: ", i, j);
        maximum[i][j] = get_value(prompt, 0, 100000);
    }
    check_safety(p, r, available, allocation, maximum);
}
