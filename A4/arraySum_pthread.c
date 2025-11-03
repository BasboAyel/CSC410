#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

#define SIZE 100000000   // Same size as OpenMP version

int arr[SIZE];
long long global_sum = 0;
pthread_mutex_t lock;
int NUM_THREADS = 1;  // default

double time_diff_sec(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1000000000.0;
}

void *partial_sum(void *arg) {
    long thread_id = (long)arg;
    long chunk = SIZE / NUM_THREADS;
    long start = thread_id * chunk;
    long end = (thread_id == NUM_THREADS - 1) ? SIZE : start + chunk;

    long long local_sum = 0;
    for (long i = start; i < end; i++) {
        local_sum += arr[i];
    }

    pthread_mutex_lock(&lock);
    global_sum += local_sum;
    pthread_mutex_unlock(&lock);

    return NULL;
}

int main() {
    // Read NUM_THREADS from environment variable if provided
    char *env = getenv("NUM_THREADS");
    if (env != NULL) {
        NUM_THREADS = atoi(env);
        if (NUM_THREADS < 1) NUM_THREADS = 1;
    }

    pthread_t threads[NUM_THREADS];
    pthread_mutex_init(&lock, NULL);

    // Initialize array
    for (int i = 0; i < SIZE; i++) arr[i] = i + 1;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Create threads
    for (long t = 0; t < NUM_THREADS; t++) {
        pthread_create(&threads[t], NULL, partial_sum, (void *)t);
    }

    // Join threads
    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("Total Sum: %lld\n", global_sum);
    printf("Pthread Time with %d threads: %f seconds\n", NUM_THREADS, time_diff_sec(start, end));

    pthread_mutex_destroy(&lock);
    return 0;
}
