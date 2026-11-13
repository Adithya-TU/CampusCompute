# CampusCompute: Intelligent Job Scheduling and Resource Management

A Linux C command-line prototype demonstrating core Operating Systems concepts.

## Requirements
- Ubuntu Linux or WSL Ubuntu
- GCC
- GNU Make
- POSIX threads, shared memory, named semaphores and process APIs (provided by Linux)

Install build tools if needed:
```bash
sudo apt update
sudo apt install build-essential
```

## Build
From the project directory:
```bash
make clean
make
```

## Run
```bash
./campuscompute
```

## Menu
1. **Job management and CPU scheduling** — enter jobs with arrival time, CPU burst and priority. Displays FCFS, non-preemptive Priority and Round Robin results, including completion time, turnaround time, waiting time and averages.
2. **Banker's safety algorithm** — enter Available, Allocation and Maximum; computes Need and reports a safe sequence or unsafe state.
3. **Concurrent activity logger** — two producer threads and one consumer use a bounded queue protected by a mutex and condition variables. Messages are printed and appended to `logs/campuscompute.log`. Threads are joined before returning.
4. **Shared-memory IPC + named semaphore** — parent and child processes exchange data through POSIX shared memory. A named semaphore coordinates access. IPC objects are cleaned up after the demonstration.
5. **Worker process and failure detection** — forks a child and uses `waitpid()` to report normal completion or an explicitly simulated nonzero exit.
6. **Process resource-limit demonstration** — uses `getrlimit()` and `setrlimit()` to lower the current process's soft `RLIMIT_NOFILE` limit. The limit affects the running CampusCompute process until it exits.

## Scheduling rules
- FCFS: jobs are ordered by arrival time, then lower job ID.
- Non-preemptive Priority: lower numeric value means higher priority; ties use earlier arrival, then lower job ID.
- Round Robin: time quantum is entered by the user. Jobs are considered in ascending job ID order on each pass; arrivals are respected.
- Turnaround time = completion time - arrival time.
- Waiting time = turnaround time - CPU burst.

## Cleanup
Exit the application with menu option `0`, then run:
```bash
make clean
```
The IPC demonstration unlinks only the named shared-memory object and named semaphore it creates. The logger file remains as project output in `logs/`.

## Notes
- Worker failure is deliberately simulated; it is not an actual system crash.
- Resource limit changes are process-local and disappear when the program exits.
- This is an educational prototype, not a production scheduler or resource manager.
