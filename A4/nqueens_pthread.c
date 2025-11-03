#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#define N 15   // << You requested N = 15
int NUM_THREADS = 1;

long long total_solutions = 0;
pthread_mutex_t lock;

bool isSafe(int board[], int row, int col) {
    for (int i = 0; i < col; i++) {
        if (board[i] == row || abs(board[i] - row) == abs(i - col))
            return false;
    }
    return true;
}

void solveNQueens(int board[], int col, long long *local_count) {
    if (col == N) {
        (*local_count)++;
        return;
    }

    for (int row = 0; row < N; row++) {
        if (isSafe(board, row, col)) {
            board[col] = row;
            solveNQueens(board, col + 1, local_count);
            board[col] = -1;
        }
    }
}

void *thread_solve(void *arg) {
    long thread_id = (long)arg;
    
    long long local_count = 0;
    int board[N];

    for (int i = 0; i < N; i++) board[i] = -1;

    // Split work: each thread handles specific first rows
    for (int row = thread_id; row < N; row += NUM_THREADS) {
        board[0] = row;
        solveNQueens(board, 1, &local_count);
    }

    pthread_mutex_lock(&lock);
    total_solutions += local_count;
    pthread_mutex_unlock(&lock);

    return NULL;
}

double time_diff_sec(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1000000000.0;
}

int main() {
    // Read # of threads from environment variable if provided
    char *env = getenv("NUM_THREADS");
    if (env != NULL) {
        NUM_THREADS = atoi(env);
        if (NUM_THREADS < 1) NUM_THREADS = 1;
    }

    pthread_t threads[NUM_THREADS];
    pthread_mutex_init(&lock, NULL);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Create threads
    for (long t = 0; t < NUM_THREADS; t++) {
        pthread_create(&threads[t], NULL, thread_solve, (void *)t);
    }

    // Join threads
    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("%d-Queens Solutions: %lld\n", N, total_solutions);
    printf("Pthread Time with %d threads: %f seconds\n",
           NUM_THREADS, time_diff_sec(start, end));

    pthread_mutex_destroy(&lock);
    return 0;
}
