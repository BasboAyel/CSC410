#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define SIZE 100000000
#define NUM_THREADS 4

long long arr[SIZE];
long long partialSums[NUM_THREADS] = {0};

void* sumPart(void* arg)
{
    int thread_id = *(int*)arg;
    long long start = (SIZE / NUM_THREADS) * thread_id;
    long long end = (thread_id == NUM_THREADS - 1) ? SIZE : (SIZE / NUM_THREADS) * (thread_id + 1);

    for (long long i = start; i < end; i++)
        partialSums[thread_id] += arr[i];

    pthread_exit(NULL);
}

int main()
{
    for (long long i = 0; i < SIZE; i++)
        arr[i] = 1; // fill array with 1s for simplicity

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    clock_t start = clock(); // Start timing

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, sumPart, &thread_ids[i]);
    }

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    long long totalSum = 0;
    for (int i = 0; i < NUM_THREADS; i++)
        totalSum += partialSums[i];

    clock_t end = clock(); // End timing

    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Total sum = %lld\n", totalSum);
    printf("Time: %f seconds\n", time_spent);

    return 0;
}
