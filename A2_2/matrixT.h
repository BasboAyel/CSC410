#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 1000          // Matrix size (NxN)
#define NUM_THREADS 4   // Number of threads

int **A, **B, **C;      // Global matrices

// Structure for each thread’s work info
typedef struct 
{
    int thread_id;
    int num_rows;       // Number of rows each thread will handle
} thread_data_t;

// Thread routine for partial matrix multiplication
void* matrixMultiplyThread(void* arg) 
{
    thread_data_t* data = (thread_data_t*)arg;
    int thread_id = data->thread_id;
    int rows_per_thread = data->num_rows;

    int start_row = thread_id * rows_per_thread;
    int end_row = (thread_id == NUM_THREADS - 1) ? N : start_row + rows_per_thread;

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    pthread_exit(NULL);
}

// Utility function to print a matrix
void displayMatrix(int** matrix, int n) 
{
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}
