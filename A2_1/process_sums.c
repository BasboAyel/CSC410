// parallel_sums.c
// Parallel array sum with selectable number of processes

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_PROCESSES 4

int main(int argc, char *argv[]) 
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <N> <num_processes>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int num_processes = atoi(argv[2]);

    if (num_processes < 1 || num_processes > MAX_PROCESSES) {
        fprintf(stderr, "num_processes must be between 1 and %d\n", MAX_PROCESSES);
        return 1;
    }

    int *arr = malloc(N * sizeof(int));
    if (!arr) {
        perror("malloc");
        return 1;
    }

    // Initialize array [1..N]
    for (int i = 0; i < N; i++) {
        arr[i] = i + 1;
    }

    int chunk = N / num_processes;
    int pipes[MAX_PROCESSES][2];
    pid_t pid;

    for (int i = 0; i < num_processes; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return 1;
        }

        pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }
        else if (pid == 0) {
            // ---- Child process ----
            close(pipes[i][0]); // close read end

            long long partial_sum = 0;
            int start = i * chunk;
            int end = (i == num_processes - 1) ? N : start + chunk;

            for (int j = start; j < end; j++) {
                partial_sum += arr[j];
            }

            // Send partial sum to parent
            write(pipes[i][1], &partial_sum, sizeof(partial_sum));
            close(pipes[i][1]);

            free(arr);  // free child’s copy
            exit(0);
        }
        else {
            // ---- Parent process ----
            close(pipes[i][1]); // close write end
        }
    }

    // ---- Parent collects results ----
    long long total = 0;
    for (int i = 0; i < num_processes; i++) {
        long long part = 0;
        read(pipes[i][0], &part, sizeof(part));
        close(pipes[i][0]);
        total += part;
    }

    // Wait for all children
    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    printf("Total sum = %lld (using %d process(es))\n", total, num_processes);

    free(arr);
    return 0;
}
