#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

#define N 100000000   // 100 million intervals
#define A 0.0
#define B 1.0

int NUM_THREADS = 1;

double f(double x) {
    return 4.0 / (1.0 + x * x);  // function to compute π
}

double time_diff_sec(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1000000000.0;
}

void *thread_integrate(void *arg) {
    long thread_id = (long)arg;
    long chunk = N / NUM_THREADS;
    long start = thread_id * chunk;
    long end = (thread_id == NUM_THREADS - 1) ? N : start + chunk;

    double h = (B - A) / N;
    double local_sum = 0.0;

    for (long i = start; i < end; i++) {
        double x = A + (i + 0.5) * h;
        local_sum += f(x);
    }

    double *result = malloc(sizeof(double));
    *result = local_sum * h;
    return result;
}

int main() {
    // Read thread count from environment variable
    char *env = getenv("NUM_THREADS");
    if (env != NULL) {
        NUM_THREADS = atoi(env);
        if (NUM_THREADS < 1) NUM_THREADS = 1;
    }

    pthread_t threads[NUM_THREADS];

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Create threads
    for (long t = 0; t < NUM_THREADS; t++) {
        pthread_create(&threads[t], NULL, thread_integrate, (void *)t);
    }

    double total = 0.0;

    // Join threads and sum their results
    for (int t = 0; t < NUM_THREADS; t++) {
        double *res;
        pthread_join(threads[t], (void **)&res);
        total += *res;
        free(res);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("Pthread Integration Result: %.12f\n", total);
    printf("Pthread Time with %d threads: %f seconds\n", 
           NUM_THREADS, time_diff_sec(start, end));

    return 0;
}
