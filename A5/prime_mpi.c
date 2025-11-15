#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <mpi.h>

#define N 10000000

int main(int argc, char *argv[]) {
    int rank, size;
    double start_time, end_time;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Calculate each process's segment of the sieve
    int chunk_size = (N + 1) / size;
    int low = rank * chunk_size;
    int high = (rank == size - 1) ? N : (low + chunk_size - 1);
    int segment_size = high - low + 1;
    
    // Allocate local prime array
    bool *is_prime = (bool *)malloc(segment_size * sizeof(bool));
    for (int i = 0; i < segment_size; i++) {
        is_prime[i] = true;
    }
    
    // Mark 0 and 1 as not prime in appropriate segments
    if (low <= 0) is_prime[0 - low] = false;
    if (low <= 1) is_prime[1 - low] = false;
    
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    
    // First, root process finds primes up to sqrt(N) for sieving
    int sqrt_n = (int)sqrt(N);
    bool *base_primes = NULL;
    
    if (rank == 0) {
        base_primes = (bool *)malloc((sqrt_n + 1) * sizeof(bool));
        for (int i = 0; i <= sqrt_n; i++) base_primes[i] = true;
        base_primes[0] = base_primes[1] = false;
        
        for (int p = 2; p <= sqrt_n; p++) {
            if (base_primes[p]) {
                for (int i = p * p; i <= sqrt_n; i += p) {
                    base_primes[i] = false;
                }
            }
        }
    }
    
    // Each process sieves its segment using base primes
    for (int p = 2; p <= sqrt_n; p++) {
        // Check if p is prime (only root knows this initially)
        int is_p_prime;
        if (rank == 0) {
            is_p_prime = base_primes[p];
        }
        MPI_Bcast(&is_p_prime, 1, MPI_INT, 0, MPI_COMM_WORLD);
        
        if (!is_p_prime) continue;
        
        // Find first multiple of p in this segment
        int first_multiple = (low / p) * p;
        if (first_multiple < low) first_multiple += p;
        if (first_multiple < p * p) first_multiple = p * p;
        
        // Mark multiples of p in this segment
        for (int i = first_multiple; i <= high; i += p) {
            if (i >= low) {
                is_prime[i - low] = false;
            }
        }
    }
    
    // Count primes in local segment
    int local_count = 0;
    for (int i = 0; i < segment_size; i++) {
        if (is_prime[i] && (i + low) >= 2) {
            local_count++;
        }
    }
    
    // Sum all local counts to get total
    int total_count;
    MPI_Reduce(&local_count, &total_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    
    if (rank == 0) {
        printf("Number of primes up to %d: %d\n", N, total_count);
        printf("Time: %f seconds\n", end_time - start_time);
        free(base_primes);
    }
    
    free(is_prime);
    MPI_Finalize();
    return 0;
}