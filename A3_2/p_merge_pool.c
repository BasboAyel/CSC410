// p_merge_pool.c — Approach 3: Thread-pool parallel merge sort
// Compile:  gcc -O2 -pthread main_merge.c p_merge_pool.c -o merge_pool
// Run:      NUM_THREADS=4 ./merge_pool

#include "p_merge.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------- helpers (same as before) ----------
void printArray(int* arr, int n) {
    for (int i = 0; i < n; ++i) printf("%d ", arr[i]);
    printf("\n");
}

double time_diff_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_nsec - start.tv_nsec) / 1000000.0;
}

void merge(int* arr, int l, int m, int r) {
    int n1 = m - l + 1, n2 = r - m;
    int* L = malloc(n1 * sizeof(int));
    int* R = malloc(n2 * sizeof(int));
    for (int i = 0; i < n1; i++) L[i] = arr[l + i];
    for (int j = 0; j < n2; j++) R[j] = arr[m + 1 + j];
    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
    free(L); free(R);
}

void merge_sort_seq(int* arr, int l, int r) {
    if (l >= r) return;
    int m = (l + r) / 2;
    merge_sort_seq(arr, l, m);
    merge_sort_seq(arr, m + 1, r);
    merge(arr, l, m, r);
}

// ---------- thread-pool structures ----------

typedef struct task {
    int* arr;
    int l, r;
    struct task* next;
} task_t;

static task_t* task_head = NULL;
static pthread_mutex_t q_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  q_haswork = PTHREAD_COND_INITIALIZER;
static int active_workers = 0;
static int stop_pool = 0;

// push a new task to queue
static void enqueue_task(int* arr, int l, int r) {
    task_t* t = malloc(sizeof(task_t));
    t->arr = arr; t->l = l; t->r = r; t->next = NULL;
    pthread_mutex_lock(&q_lock);
    if (!task_head) task_head = t;
    else {
        task_t* p = task_head;
        while (p->next) p = p->next;
        p->next = t;
    }
    pthread_cond_signal(&q_haswork);
    pthread_mutex_unlock(&q_lock);
}

// pop a task from queue
static task_t* dequeue_task(void) {
    pthread_mutex_lock(&q_lock);
    while (!task_head && !stop_pool)
        pthread_cond_wait(&q_haswork, &q_lock);
    task_t* t = task_head;
    if (t) task_head = t->next;
    pthread_mutex_unlock(&q_lock);
    return t;
}

static void* worker_func(void* arg);

// spawn pool of N threads
static pthread_t* start_pool(int num_threads) {
    pthread_t* th = malloc(num_threads * sizeof(pthread_t));
    for (int i = 0; i < num_threads; ++i)
        pthread_create(&th[i], NULL, worker_func, NULL);
    return th;
}

// graceful shutdown
static void stop_all(pthread_t* th, int num_threads) {
    pthread_mutex_lock(&q_lock);
    stop_pool = 1;
    pthread_cond_broadcast(&q_haswork);
    pthread_mutex_unlock(&q_lock);
    for (int i = 0; i < num_threads; ++i)
        pthread_join(th[i], NULL);
    free(th);
}

// ---------- recursive divide but schedule to pool ----------
#define MIN_CHUNK 2048

static void schedule_sort(int* arr, int l, int r) {
    int n = r - l + 1;
    if (n <= MIN_CHUNK) { merge_sort_seq(arr, l, r); return; }
    enqueue_task(arr, l, r);
}

// worker routine
static void* worker_func(void* arg) {
    (void)arg;
    for (;;) {
        task_t* t = dequeue_task();
        if (!t) {
            if (stop_pool) break;
            else continue;
        }
        int l = t->l, r = t->r;
        int n = r - l + 1;
        if (n <= MIN_CHUNK) {
            merge_sort_seq(t->arr, l, r);
            free(t);
            continue;
        }
        int m = (l + r) / 2;
        // schedule sub-tasks
        enqueue_task(t->arr, l, m);
        enqueue_task(t->arr, m + 1, r);
        // sort halves, then merge after they are roughly done
        merge_sort_seq(t->arr, l, m);
        merge_sort_seq(t->arr, m + 1, r);
        merge(t->arr, l, m, r);
        free(t);
    }
    return NULL;
}

// ---------- main entry point ----------
void merge_sort_p(int* arr, int l, int r) {
    int num_threads = 4;
    const char* s = getenv("NUM_THREADS");
    if (s) {
        int v = atoi(s);
        if (v > 0 && v <= 64) num_threads = v;
    }

    pthread_t* pool = start_pool(num_threads);

    // initial top-level task
    enqueue_task(arr, l, r);

    // wait until queue empties (simple polling)
    for (;;) {
        pthread_mutex_lock(&q_lock);
        int empty = (task_head == NULL);
        pthread_mutex_unlock(&q_lock);
        if (empty) break;
        sched_yield();
    }

    stop_all(pool, num_threads);
}
