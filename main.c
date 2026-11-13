#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduling.h"
#include "banker.h"
#include "logger.h"
#include "ipc.h"
#include "worker.h"

static int read_choice(void) {
    char line[128], extra;
    int choice;
    for (;;) {
        printf("\nCampusCompute - OS Concepts Demonstrator\n");
        printf("1. Job management and CPU scheduling\n");
        printf("2. Banker's safety algorithm\n");
        printf("3. Concurrent activity logger (POSIX threads)\n");
        printf("4. Shared-memory IPC + named semaphore\n");
        printf("5. Worker process and failure detection\n");
        printf("6. Process resource-limit demonstration\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        if (!fgets(line, sizeof(line), stdin)) return 0;
        if (sscanf(line, " %d %c", &choice, &extra) == 1 && choice >= 0 && choice <= 6)
            return choice;
        printf("Invalid choice. Enter a number from 0 to 6.\n");
    }
}

int main(void) {
    int choice;
    do {
        choice = read_choice();
        switch (choice) {
            case 1: run_scheduling(); break;
            case 2: run_banker(); break;
            case 3: run_logger(); break;
            case 4: run_ipc(); break;
            case 5: run_worker(); break;
            case 6: run_resource_limit(); break;
            case 0: printf("CampusCompute shutting down. Goodbye.\n"); break;
            default: break;
        }
    } while (choice != 0);
    return 0;
}
