#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS 4
#define INTERVALS 1000000000  // Number of intervals
#define LOWER 0.0
#define UPPER 1.0

typedef struct {
    int id;
    double result;
} thread_data_t;

double f(double x) {
    return 4.0 / (1.0 + x * x);  // Function for π integration
}

double time_diff_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_nsec - start.tv_nsec) / 1000000.0;
}

void *integrate(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int id = data->id;
    long chunk = INTERVALS / NUM_THREADS;
    double step = (UPPER - LOWER) / INTERVALS;
    double local_sum = 0.0;

    long start = id * chunk;
    long end = (id == NUM_THREADS - 1) ? INTERVALS : (id + 1) * chunk;

    for (long i = start; i < end; i++) {
        double x = LOWER + (i + 0.5) * step;
        local_sum += f(x);
    }

    data->result = local_sum * step;
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    thread_data_t thread_data[NUM_THREADS];
    struct timespec start, end;

    clock_gettime(CLOCK_REALTIME, &start);

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].id = i;
        pthread_create(&threads[i], NULL, integrate, &thread_data[i]);
    }

    double total = 0.0;
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        total += thread_data[i].result;
    }

    clock_gettime(CLOCK_REALTIME, &end);

    printf("Parallel Integration Time: %.3f ms\n", time_diff_ms(start, end));
    printf("Result of numerical integration: %.10f\n", total);

    return 0;
}
