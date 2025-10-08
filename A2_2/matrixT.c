#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define SIZE 1000
#define NUM_THREADS 4

int A[SIZE][SIZE], B[SIZE][SIZE], C[SIZE][SIZE];

void* multiplyPart(void* arg)
{
    int thread_id = *(int*)arg;
    int start = (SIZE / NUM_THREADS) * thread_id;
    int end = (thread_id == NUM_THREADS - 1) ? SIZE : (SIZE / NUM_THREADS) * (thread_id + 1);

    for (int i = start; i < end; i++) {
        for (int j = 0; j < SIZE; j++) {
            C[i][j] = 0;
            for (int k = 0; k < SIZE; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    pthread_exit(NULL);
}

int main()
{
    // Initialize matrices
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            A[i][j] = 1;
            B[i][j] = 1;
        }
    }

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    clock_t start = clock(); // Start timing

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, multiplyPart, &thread_ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    clock_t end = clock(); // End timing
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Matrix multiplication done.\n");
    printf("Time: %f seconds\n", time_spent);

    return 0;
}
