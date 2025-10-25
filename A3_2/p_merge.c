#include "p_merge.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// ---------------------------------------------------------------------
// Helper functions

// Print array (used only for small SIZE or debugging)
void printArray(int* arr, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

// Compute elapsed time in milliseconds (returns double)
double time_diff_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_nsec - start.tv_nsec) / 1000000.0;
}

// Merge two sorted halves
void merge(int* arr, int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;

    int* L = malloc(n1 * sizeof(int));
    int* R = malloc(n2 * sizeof(int));

    for (int i = 0; i < n1; i++) L[i] = arr[l + i];
    for (int j = 0; j < n2; j++) R[j] = arr[m + 1 + j];

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) arr[k++] = L[i++];
        else arr[k++] = R[j++];
    }

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    free(L);
    free(R);
}

// Sequential merge sort (used for comparison and small chunks)
void merge_sort_seq(int* arr, int l, int r) {
    if (l >= r) return;
    int m = (l + r) / 2;
    merge_sort_seq(arr, l, m);
    merge_sort_seq(arr, m + 1, r);
    merge(arr, l, m, r);
}

// ---------------------------------------------------------------------
// Parallel merge sort implementation

#define MAX_LEVEL 4
#define MIN_CHUNK 2048

typedef struct {
    int* arr;
    int l, r;
    int level;
} work_t;

void* parallel_merge_sort(void* arg) {
    work_t* w = (work_t*)arg;
    int l = w->l, r = w->r, level = w->level;
    int n = r - l + 1;

    if (l >= r) return NULL;

    // Stop creating new threads if recursion depth or chunk size limit reached
    if (level >= MAX_LEVEL || n <= MIN_CHUNK) {
        merge_sort_seq(w->arr, l, r);
        return NULL;
    }

    int m = (l + r) / 2;

    pthread_t tleft, tright;
    work_t left  = { w->arr, l,   m,     level + 1 };
    work_t right = { w->arr, m+1, r,     level + 1 };

    int rc1 = pthread_create(&tleft,  NULL, parallel_merge_sort, &left);
    int rc2 = pthread_create(&tright, NULL, parallel_merge_sort, &right);

    if (rc1 == 0) pthread_join(tleft, NULL);
    else merge_sort_seq(w->arr, l, m);

    if (rc2 == 0) pthread_join(tright, NULL);
    else merge_sort_seq(w->arr, m+1, r);

    merge(w->arr, l, m, r);
    return NULL;
}

// Wrapper function called from main_merge.c
void merge_sort_p(int* arr, int l, int r) {
    work_t root = { arr, l, r, 0 };
    parallel_merge_sort(&root);
}
