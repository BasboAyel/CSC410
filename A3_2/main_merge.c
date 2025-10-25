// Enable clock_gettime(CLOCK_MONOTONIC) on POSIX
#define _POSIX_C_SOURCE 199309L

#include "p_merge.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Small wrapper so we always get a timestamp even on stricter libcs
static void now_mono(struct timespec* ts) {
#ifdef CLOCK_MONOTONIC
    clock_gettime(CLOCK_MONOTONIC, ts);
#else
    // Fallback (not strictly monotonic, but keeps code compiling)
    timespec_get(ts, TIME_UTC);
#endif
}

int main(void)
{
    struct timespec start, end;

    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    if (!arr1 || !arr2) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    printf("Generating random array of size %d...\n", SIZE);

    // Seed RNG and fill both arrays with identical values
    srand((unsigned)time(NULL));
    for (int i = 0; i < SIZE; i++) {
        int val = rand() % MAX_VAL;
        arr1[i] = val;
        arr2[i] = val;
    }

    // ----------------- Sequential -----------------
    now_mono(&start);
    merge_sort_seq(arr1, 0, SIZE - 1);
    now_mono(&end);
    printf("Sequential Merge Sort Time: %.3f ms\n", time_diff_ms(start, end));

    // Commented out to avoid huge output during benchmarking
    // printArray(arr1, SIZE);

    // ------------------ Parallel ------------------
    now_mono(&start);
    merge_sort_p(arr2, 0, SIZE - 1);
    now_mono(&end);
    printf("Parallel Merge Sort Time: %.3f ms\n", time_diff_ms(start, end));

    // Commented out to avoid huge output during benchmarking
    // printArray(arr2, SIZE);

    free(arr1);
    free(arr2);
    return 0;
}
