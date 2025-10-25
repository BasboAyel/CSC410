// p_merge_seg.c  — Approach 1 (Segment-based)
// Compile:  gcc -O2 -pthread main_merge.c p_merge_seg.c -o merge_seg
// Run:      NUM_THREADS=1 ./merge_seg   (or 2/3/4)

#include "p_merge.h"
#include <pthread.h>
#include <string.h>

// ---------- helpers (same signatures as your header expects) ----------
void printArray(int* arr, int n) {
    for (int i = 0; i < n; ++i) printf("%d ", arr[i]);
    printf("\n");
}

double time_diff_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0
         + (end.tv_nsec - start.tv_nsec) / 1000000.0;
}

// Standard merge used by both sequential and the final k-way merge
void merge(int* arr, int l, int m, int r) {
    int n1 = m - l + 1, n2 = r - m;
    int *L = (int*)malloc(n1 * sizeof(int));
    int *R = (int*)malloc(n2 * sizeof(int));
    if (!L || !R) { perror("malloc"); exit(1); }

    for (int i = 0; i < n1; ++i) L[i] = arr[l + i];
    for (int j = 0; j < n2; ++j) R[j] = arr[m + 1 + j];

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    free(L); free(R);
}

// Plain sequential merge sort (used per-segment)
void merge_sort_seq(int* arr, int l, int r) {
    if (l >= r) return;
    int m = (l + r) / 2;
    merge_sort_seq(arr, l, m);
    merge_sort_seq(arr, m + 1, r);
    merge(arr, l, m, r);
}

// ---------- Approach 1 implementation ----------

typedef struct {
    int *arr;
    int left;
    int right;
} seg_t;

static int get_num_threads(void) {
    const char* s = getenv("NUM_THREADS");
    int n = s ? atoi(s) : 4;            // default 4 for your benchmark
    if (n < 1) n = 1;
    if (n > 64) n = 64;                  // sanity cap
    return n;
}

// worker: sort ONLY its assigned segment using the sequential merge sort
static void* sort_segment(void* arg) {
    seg_t* seg = (seg_t*)arg;
    merge_sort_seq(seg->arr, seg->left, seg->right);
    return NULL;
}

// After all segments are sorted, iteratively merge adjacent runs:
// width = segment_size, then 2*segment_size, 4*segment_size, ...
static void merge_all_segments(int* arr, int n, int num_threads) {
    int base = n / num_threads;
    int rem  = n % num_threads;

    // segment boundaries we produced:
    // seg i covers [start[i], end[i]] inclusive
    int* start = (int*)malloc(num_threads * sizeof(int));
    int* end   = (int*)malloc(num_threads * sizeof(int));
    if (!start || !end) { perror("malloc"); exit(1); }

    int idx = 0;
    for (int t = 0; t < num_threads; ++t) {
        int count = base + (t < rem ? 1 : 0);
        start[t] = idx;
        end[t]   = idx + count - 1;
        idx += count;
    }

    // Iteratively merge adjacent segments until only one remains.
    // Each round halves the number of runs.
    int runs = num_threads;
    while (runs > 1) {
        int new_runs = 0;
        for (int i = 0; i + 1 < runs; i += 2) {
            int l = start[i];
            int r = end[i + 1];
            int m = end[i];
            merge(arr, l, m, r);

            // collapse i and i+1 into one run [l..r]
            start[new_runs] = l;
            end[new_runs]   = r;
            new_runs++;
        }
        if (runs % 2 == 1) {
            // carry over the last odd run
            start[new_runs] = start[runs - 1];
            end[new_runs]   = end[runs - 1];
            new_runs++;
        }
        runs = new_runs;
    }

    free(start);
    free(end);
}

// Public entry point used by main_merge.c
// Partitions array into N segments, sorts each in its OWN thread, then merges.
void merge_sort_p(int* arr, int l, int r) {
    int n = r - l + 1;
    int num_threads = get_num_threads();
    if (num_threads > n) num_threads = n;   // more threads than items is pointless

    // Build segment list
    int base = n / num_threads;
    int rem  = n % num_threads;

    pthread_t *th = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    seg_t     *sg = (seg_t*)malloc(num_threads * sizeof(seg_t));
    if (!th || !sg) { perror("malloc"); exit(1); }

    int idx = l;
    for (int t = 0; t < num_threads; ++t) {
        int count = base + (t < rem ? 1 : 0);
        sg[t].arr   = arr;
        sg[t].left  = idx;
        sg[t].right = idx + count - 1;
        idx += count;
        pthread_create(&th[t], NULL, sort_segment, &sg[t]);
    }

    for (int t = 0; t < num_threads; ++t) pthread_join(th[t], NULL);

    // Merge the sorted segments into a single sorted array
    merge_all_segments(arr, n, num_threads);

    free(th);
    free(sg);
}
