#include <stdio.h>
#include <mpi.h>

#define N 1000000000  // 1,000,000,000 intervals as required

double f(double x) {
    return 4.0 / (1.0 + x * x);
}

int main(int argc, char *argv[]) {
    int rank, size;
    double start_time, end_time;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    double a = 0.0, b = 1.0;
    double h = (b - a) / N;
    
    // Calculate local range for trapezoidal rule
    int local_n = N / size;
    int start = rank * local_n;
    int end = (rank == size - 1) ? N : start + local_n;
    
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    
    // Trapezoidal rule calculation
    double local_sum = 0.0;
    
    // Handle endpoints correctly
    if (rank == 0) {
        local_sum = f(a) + f(b);  // First process handles endpoints
    }
    
    // Sum the middle terms (each term counted twice in trapezoidal rule)
    for (int i = start; i < end; i++) {
        if (i == 0 || i == N) continue;  // Skip endpoints handled by rank 0
        double x = a + i * h;
        local_sum += 2.0 * f(x);
    }
    
    // Sum all local sums
    double total_sum;
    MPI_Reduce(&local_sum, &total_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    end_time = MPI_Wtime();
    
    if (rank == 0) {
        double pi_estimate = (h / 2.0) * total_sum;
        printf("Estimated value of π: %.15f\n", pi_estimate);
        printf("Elapsed time: %f seconds\n", end_time - start_time);
    }
    
    MPI_Finalize();
    return 0;
}