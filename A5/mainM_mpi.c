#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "matrixMul.h"

int main(int argc, char** argv)
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start, end;

    // Each process will allocate its own blocks
    int rows_per_proc = N / size;
    int extra = N % size;  // if N is not divisible by size

    int start_row, end_row;

    if (rank < extra) {
        start_row = rank * (rows_per_proc + 1);
        end_row   = start_row + (rows_per_proc + 1);
    } else {
        start_row = rank * rows_per_proc + extra;
        end_row   = start_row + rows_per_proc;
    }

    int local_rows = end_row - start_row;  // how many rows this process computes

    // Allocate local A block and full B and C blocks
    int** A = (int**)malloc(local_rows * sizeof(int*));
    int** B = (int**)malloc(N * sizeof(int*));
    int** C = (int**)malloc(local_rows * sizeof(int*));

    for (int i = 0; i < local_rows; i++)
        A[i] = (int*)malloc(N * sizeof(int));

    for (int i = 0; i < N; i++)
        B[i] = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < local_rows; i++)
        C[i] = (int*)malloc(N * sizeof(int));

    // Rank 0 initializes full matrices A_full and B_full
    int** A_full = NULL;
    int** B_full = NULL;

    if (rank == 0) {
        A_full = (int**)malloc(N * sizeof(int*));
        B_full = (int**)malloc(N * sizeof(int*));

        for (int i = 0; i < N; i++) {
            A_full[i] = (int*)malloc(N * sizeof(int));
            B_full[i] = (int*)malloc(N * sizeof(int));
        }

        // Initialize matrices
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                A_full[i][j] = 1;
                B_full[i][j] = 1;
            }
        }
    }

    // Broadcast matrix B to all processes
    for (int i = 0; i < N; i++)
        MPI_Bcast(B[i], N, MPI_INT, 0, MPI_COMM_WORLD);

    // Scatter A manually (because blocks may not be equal)
    if (rank == 0) {
        for (int p = 0; p < size; p++) {
            int p_start, p_end;
            if (p < extra) {
                p_start = p * (rows_per_proc + 1);
                p_end   = p_start + (rows_per_proc + 1);
            } else {
                p_start = p * rows_per_proc + extra;
                p_end   = p_start + rows_per_proc;
            }
            int p_rows = p_end - p_start;

            if (p == 0) {
                // copy for self
                for (int r = 0; r < p_rows; r++)
                    for (int c = 0; c < N; c++)
                        A[r][c] = A_full[p_start + r][c];
            } else {
                for (int r = 0; r < p_rows; r++)
                    MPI_Send(A_full[p_start + r], N, MPI_INT, p, 0, MPI_COMM_WORLD);
            }
        }
    } else {
        for (int r = 0; r < local_rows; r++)
            MPI_Recv(A[r], N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();

    // Perform local multiplication: C_local = A_local × B
    for (int i = 0; i < local_rows; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < N; k++)
                sum += A[i][k] * B[k][j];
            C[i][j] = sum;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();

    if (rank == 0)
        printf("MPI Matrix Multiplication Time: %f seconds\n", end - start);

    // Gather results back to rank 0
    if (rank == 0) {
        int offset = 0;
        // Copy my rows
        for (int r = 0; r < local_rows; r++)
            for (int c = 0; c < N; c++)
                A_full[r][c] = C[r][c];

        // Receive from others
        for (int p = 1; p < size; p++) {
            int p_start, p_end;
            if (p < extra) {
                p_start = p * (rows_per_proc + 1);
                p_end   = p_start + (rows_per_proc + 1);
            } else {
                p_start = p * rows_per_proc + extra;
                p_end   = p_start + rows_per_proc;
            }
            int p_rows = p_end - p_start;

            for (int r = 0; r < p_rows; r++)
                MPI_Recv(A_full[p_start + r], N, MPI_INT, p, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        printf("MPI Matrix multiplication complete.\n");
    } else {
        for (int r = 0; r < local_rows; r++)
            MPI_Send(C[r], N, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
