// nqueens.c
// Parallel N-Queens using pthreads
// Choose number of threads with: ./a.out <num_threads>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>

#define N 15
#define MAX_THREADS 64

int total_solutions = 0;
int NUM_THREADS = 1;
pthread_mutex_t count_mutex;

// Struct to hold partial board state for each thread
typedef struct {
    int board[N];
    int col;
} ThreadData;

// Check if it's safe to place a queen at board[row][col]
bool isSafe(int board[], int row, int col) {
    for (int i = 0; i < col; i++) {
        if (board[i] == row || abs(board[i] - row) == abs(i - col))
            return false;
    }
    return true;
}

// Recursive backtracking solver
void solve(int board[], int col, int *local_count) {
    if (col == N) {
        (*local_count)++;
        return;
    }

    for (int row = 0; row < N; row++) {
        if (isSafe(board, row, col)) {
            board[col] = row;
            solve(board, col + 1, local_count);
        }
    }
}

// Thread function: each thread solves from a partial board state
void* thread_solve(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int local_count = 0;

    solve(data->board, data->col, &local_count);

    pthread_mutex_lock(&count_mutex);
    total_solutions += local_count;
    pthread_mutex_unlock(&count_mutex);

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <num_threads>\n", argv[0]);
        return 1;
    }

    NUM_THREADS = atoi(argv[1]);
    if (NUM_THREADS <= 0 || NUM_THREADS > MAX_THREADS)
        NUM_THREADS = 1;

    pthread_t threads[MAX_THREADS];
    ThreadData thread_data[MAX_THREADS];
    pthread_mutex_init(&count_mutex, NULL);

    int board[N];
    for (int i = 0; i < N; i++)
        board[i] = -1;

    int thread_count = 0;

    // Create threads for the first column's valid queen placements
    for (int row = 0; row < N; row++) {
        if (isSafe(board, row, 0)) {
            for (int i = 0; i < N; i++)
                thread_data[thread_count].board[i] = board[i];

            thread_data[thread_count].board[0] = row;
            thread_data[thread_count].col = 1;

            pthread_create(&threads[thread_count], NULL, thread_solve, &thread_data[thread_count]);
            thread_count++;

            if (thread_count >= NUM_THREADS)
                break;
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < thread_count; i++)
        pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&count_mutex);

    printf("Total solutions for N=%d using %d thread(s): %d\n", N, NUM_THREADS, total_solutions);

    return 0;
}
