#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define G 6.67430e-11
#define NUM_BODIES 1000
#define DT (60*60*24)

typedef struct {
    double x, y;
    double vx, vy;
    double mass;
} Body;

Body bodies[NUM_BODIES];

int main() {
    for (int i = 0; i < NUM_BODIES; i++) {
        bodies[i].x = rand() % 1000000000;
        bodies[i].y = rand() % 1000000000;
        bodies[i].vx = (rand()%100 - 50) * 1e3;
        bodies[i].vy = (rand()%100 - 50) * 1e3;
        bodies[i].mass = (rand()%100 + 1) * 1e24;
    }

    for (int step = 0; step < 1000; step++) {
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < NUM_BODIES; i++) {
            double fx = 0.0, fy = 0.0;
            for (int j = 0; j < NUM_BODIES; j++) {
                if (i != j) {
                    double dx = bodies[j].x - bodies[i].x;
                    double dy = bodies[j].y - bodies[i].y;
                    double dist = sqrt(dx*dx + dy*dy);
                    if (dist > 0) {
                        double f = G * bodies[i].mass * bodies[j].mass / (dist*dist);
                        fx += f * dx / dist;
                        fy += f * dy / dist;
                    }
                }
            }
            bodies[i].vx += fx / bodies[i].mass * DT;
            bodies[i].vy += fy / bodies[i].mass * DT;
        }

        #pragma omp parallel for
        for (int i = 0; i < NUM_BODIES; i++) {
            bodies[i].x += bodies[i].vx * DT;
            bodies[i].y += bodies[i].vy * DT;
        }
    }
    return 0;
}
