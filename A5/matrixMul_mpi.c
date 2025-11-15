#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N 1000

int main(int argc, char *argv[]) {
    int rank, size;
    int **A, **B, **C;
    double start_time, end_time;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Calculate local rows
    int rows_per_proc = N / size;
    int start_row = rank * rows_per_proc;
    int end_row = (rank == size - 1) ? N : start_row + rows_per_proc;
    int local_rows = end_row - start_row;
    
    // Allocate memory
    A = (int **)malloc(local_rows * sizeof(int *));
    B = (int **)malloc(N * sizeof(int *));
    C = (int **)malloc(local_rows * sizeof(int *));
    
    for (int i = 0; i < local_rows; i++) {
        A[i] = (int *)malloc(N * sizeof(int));
        C[i] = (int *)malloc(N * sizeof(int));
    }
    for (int i = 0; i < N; i++) {
        B[i] = (int *)malloc(N * sizeof(int));
    }
    
    // Initialize matrices
    for (int i = 0; i < local_rows; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = 1;
        }
    }
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            B[i][j] = 1;
        }
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    
    // Local matrix multiplication
    for (int i = 0; i < local_rows; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    
    end_time = MPI_Wtime();
    
    if (rank == 0) {
        printf("Matrix Multiplication Time: %f seconds\n", end_time - start_time);
    }
    
    // Cleanup
    for (int i = 0; i < local_rows; i++) {
        free(A[i]);
        free(C[i]);
    }
    for (int i = 0; i < N; i++) {
        free(B[i]);
    }
    free(A);
    free(B);
    free(C);
    
    MPI_Finalize();
    return 0;
}