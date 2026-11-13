#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include "worker.h"

void run_worker(void) {
    int choice;
    char line[64], extra;
    for (;;) {
        printf("Worker mode: 1 = normal completion, 2 = simulated failure: ");
        if (!fgets(line, sizeof(line), stdin)) return;
        if (sscanf(line, " %d %c", &choice, &extra) == 1 && (choice == 1 || choice == 2)) break;
        printf("Enter 1 or 2.\n");
    }
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return; }
    if (pid == 0) {
        printf("Worker child PID %ld started.\n", (long)getpid());
        fflush(stdout);
        if (choice == 1) {
            printf("Worker child completed normally.\n");
            fflush(stdout);
            _exit(0);
        } else {
            fprintf(stderr, "Worker child: controlled simulated failure (exit code 2).\n");
            _exit(2);
        }
    }
    int status;
    if (waitpid(pid, &status, 0) == -1) { perror("waitpid"); return; }
    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        if (code == 0) printf("Parent detected normal worker completion (exit code 0).\n");
        else printf("Parent detected worker failure (exit code %d). This was deliberately simulated.\n", code);
    } else if (WIFSIGNALED(status)) {
        printf("Parent detected worker termination by signal %d.\n", WTERMSIG(status));
    }
}

void run_resource_limit(void) {
    struct rlimit old_limit, new_limit, observed;
    if (getrlimit(RLIMIT_NOFILE, &old_limit) == -1) { perror("getrlimit"); return; }
    new_limit = old_limit;
    rlim_t target = 32;
    if (new_limit.rlim_max != RLIM_INFINITY && target > new_limit.rlim_max)
        target = new_limit.rlim_max;
    new_limit.rlim_cur = target;
    if (setrlimit(RLIMIT_NOFILE, &new_limit) == -1) { perror("setrlimit"); return; }
    if (getrlimit(RLIMIT_NOFILE, &observed) == -1) { perror("getrlimit"); return; }
    printf("Resource limit demonstration (RLIMIT_NOFILE):\n");
    printf("Previous soft limit: %ld\n", (long)old_limit.rlim_cur);
    printf("New soft limit: %ld\n", (long)observed.rlim_cur);
    printf("This limits the number of file descriptors available to this process. The change applies to this menu process.\n");
}
