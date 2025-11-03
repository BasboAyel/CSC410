#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N 1000  // same size as OpenMP version

int **A, **B, **C;
int NUM_THREADS = 1;

double time_diff_sec(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1000000000.0;
}

void *multiply(void *arg) {
    long thread_id = (long)arg;
    
    int rows_per_thread = N / NUM_THREADS;
    int start_row = thread_id * rows_per_thread;
    int end_row = (thread_id == NUM_THREADS - 1) ? N : start_row + rows_per_thread;

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < N; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }

    return NULL;
}

int main() {
    // Read NUM_THREADS from environment variable
    char *env = getenv("NUM_THREADS");
    if (env != NULL) {
        NUM_THREADS = atoi(env);
        if (NUM_THREADS < 1) NUM_THREADS = 1;
    }

    // Allocate matrices
    A = malloc(N * sizeof(int *));
    B = malloc(N * sizeof(int *));
    C = malloc(N * sizeof(int *));
    
    for (int i = 0; i < N; i++) {
        A[i] = malloc(N * sizeof(int));
        B[i] = malloc(N * sizeof(int));
        C[i] = malloc(N * sizeof(int));
        for (int j = 0; j < N; j++) {
            A[i][j] = i + j;
            B[i][j] = i - j;
            C[i][j] = 0;
        }
    }

    pthread_t threads[NUM_THREADS];

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Create threads
    for (long t = 0; t < NUM_THREADS; t++) {
        pthread_create(&threads[t], NULL, multiply, (void *)t);
    }

    // Join threads
    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("Pthread Matrix Multiplication Time with %d threads: %f seconds\n", 
           NUM_THREADS, time_diff_sec(start, end));

    // Free memory
    for (int i = 0; i < N; i++) {
        free(A[i]); free(B[i]); free(C[i]);
    }
    free(A); free(B); free(C);

    return 0;
}
