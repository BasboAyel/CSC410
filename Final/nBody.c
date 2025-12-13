// Sequential N-body simulation (FIXED for benchmarking)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define G 6.67430e-11
#define NUM_BODIES 1000
#define DT (60*60*24)
#define STEPS 1000

typedef struct {
    double x, y;
    double vx, vy;
    double mass;
} Body;

void compute_gravitational_force(Body *b1, Body *b2, double *fx, double *fy) {
    double dx = b2->x - b1->x;
    double dy = b2->y - b1->y;
    double dist = sqrt(dx*dx + dy*dy);
    if (dist == 0.0) return;

    double f = G * b1->mass * b2->mass / (dist * dist);
    *fx += f * dx / dist;
    *fy += f * dy / dist;
}

void update_bodies(Body bodies[], int n) {
    for (int i = 0; i < n; i++) {
        double fx = 0.0, fy = 0.0;
        for (int j = 0; j < n; j++) {
            if (i != j)
                compute_gravitational_force(&bodies[i], &bodies[j], &fx, &fy);
        }
        bodies[i].vx += fx / bodies[i].mass * DT;
        bodies[i].vy += fy / bodies[i].mass * DT;
    }

    for (int i = 0; i < n; i++) {
        bodies[i].x += bodies[i].vx * DT;
        bodies[i].y += bodies[i].vy * DT;
    }
}

int main() {
    Body bodies[NUM_BODIES];

    for (int i = 0; i < NUM_BODIES; i++) {
        bodies[i].x = rand() % 1000000000;
        bodies[i].y = rand() % 1000000000;
        bodies[i].vx = 0;
        bodies[i].vy = 0;
        bodies[i].mass = (rand() % 100 + 1) * 1e24;
    }

    for (int step = 0; step < STEPS; step++) {
        update_bodies(bodies, NUM_BODIES);
    }

    // Optional: print a few bodies to verify correctness
    for (int i = 0; i < 5; i++) {
        printf("Body %d: (%.2f, %.2f)\n", i, bodies[i].x, bodies[i].y);
    }

    return 0;
}
