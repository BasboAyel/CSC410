#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define G 6.67430e-11
#define NUM_BODIES 1000
#define DT (60*60*24)
#define NUM_THREADS 8

typedef struct {
    double x, y;
    double vx, vy;
    double mass;
} Body;

Body bodies[NUM_BODIES];

typedef struct {
    int start;
    int end;
} ThreadData;

void compute_gravitational_force(Body *b1, Body *b2, double *fx, double *fy) {
    double dx = b2->x - b1->x;
    double dy = b2->y - b1->y;
    double dist = sqrt(dx*dx + dy*dy);
    if (dist == 0.0) return;
    double f = G * b1->mass * b2->mass / (dist * dist);
    *fx += f * dx / dist;
    *fy += f * dy / dist;
}

void *worker(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    for (int i = data->start; i < data->end; i++) {
        double fx = 0.0, fy = 0.0;
        for (int j = 0; j < NUM_BODIES; j++) {
            if (i != j)
                compute_gravitational_force(&bodies[i], &bodies[j], &fx, &fy);
        }
        bodies[i].vx += fx / bodies[i].mass * DT;
        bodies[i].vy += fy / bodies[i].mass * DT;
    }
    pthread_exit(NULL);
}

int main() {
    for (int i = 0; i < NUM_BODIES; i++) {
        bodies[i].x = rand() % 1000000000;
        bodies[i].y = rand() % 1000000000;
        bodies[i].vx = (rand()%100 - 50) * 1e3;
        bodies[i].vy = (rand()%100 - 50) * 1e3;
        bodies[i].mass = (rand()%100 + 1) * 1e24;
    }

    pthread_t threads[NUM_THREADS];
    ThreadData data[NUM_THREADS];
    int chunk = NUM_BODIES / NUM_THREADS;

    for (int step = 0; step < 1000; step++) {
        for (int t = 0; t < NUM_THREADS; t++) {
            data[t].start = t * chunk;
            data[t].end = (t == NUM_THREADS - 1) ? NUM_BODIES : (t+1)*chunk;
            pthread_create(&threads[t], NULL, worker, &data[t]);
        }

        for (int t = 0; t < NUM_THREADS; t++)
            pthread_join(threads[t], NULL);

        for (int i = 0; i < NUM_BODIES; i++) {
            bodies[i].x += bodies[i].vx * DT;
            bodies[i].y += bodies[i].vy * DT;
        }
    }
    return 0;
}
