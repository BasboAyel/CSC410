#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define SIZE 100000
#define MAX_VAL 100000
#define NUM_THREADS 4

int arr[SIZE];

// ---- custom barrier for macOS/Linux ----
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int tripCount;
} barrier_t;

int barrier_init(barrier_t *barrier, int count) {
    barrier->count = 0;
    barrier->tripCount = count;
    pthread_mutex_init(&barrier->mutex, NULL);
    pthread_cond_init(&barrier->cond, NULL);
    return 0;
}

void barrier_wait(barrier_t *barrier) {
    pthread_mutex_lock(&barrier->mutex);
    barrier->count++;
    if (barrier->count >= barrier->tripCount) {
        barrier->count = 0;
        pthread_cond_broadcast(&barrier->cond);
    } else {
        pthread_cond_wait(&barrier->cond, &barrier->mutex);
    }
    pthread_mutex_unlock(&barrier->mutex);
}

void barrier_destroy(barrier_t *barrier) {
    pthread_mutex_destroy(&barrier->mutex);
    pthread_cond_destroy(&barrier->cond);
}
// ----------------------------------------

barrier_t barrier;

double time_diff_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_nsec - start.tv_nsec) / 1000000.0;
}

void *bubble_thread(void *arg) {
    long tid = (long)arg;
    for (int i = 0; i < SIZE - 1; i++) {
        for (int j = tid; j < SIZE - i - 1; j += NUM_THREADS) {
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
        barrier_wait(&barrier);
    }
    return NULL;
}

int main() {
    struct timespec start, end;
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < SIZE; i++) arr[i] = rand() % MAX_VAL;

    barrier_init(&barrier, NUM_THREADS);

    clock_gettime(CLOCK_REALTIME, &start);

    for (long t = 0; t < NUM_THREADS; t++)
        pthread_create(&threads[t], NULL, bubble_thread, (void *)t);

    for (int t = 0; t < NUM_THREADS; t++)
        pthread_join(threads[t], NULL);

    clock_gettime(CLOCK_REALTIME, &end);

    printf("Parallel Bubble Sort Time: %.3f ms\n", time_diff_ms(start, end));

    barrier_destroy(&barrier);
    return 0;
}
