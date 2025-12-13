#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define G 6.67430e-11
#define NUM_BODIES 1000
#define DT (60*60*24)

typedef struct {
    double x, y, vx, vy, mass;
} Body;

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int local_n = NUM_BODIES / size;
    Body *local = malloc(local_n * sizeof(Body));
    Body *all = malloc(NUM_BODIES * sizeof(Body));

    if (rank == 0) {
        for (int i = 0; i < NUM_BODIES; i++) {
            all[i].x = rand() % 1000000000;
            all[i].y = rand() % 1000000000;


            all[i].vx = 0;
            all[i].vy = 0;
            all[i].mass = (rand()%100+1)*1e24;
        }
    }

    MPI_Scatter(all, local_n*sizeof(Body), MPI_BYTE,
                local, local_n*sizeof(Body), MPI_BYTE, 0, MPI_COMM_WORLD);

    for (int step = 0; step < 1000; step++) {
        MPI_Allgather(local, local_n*sizeof(Body), MPI_BYTE,
                      all, local_n*sizeof(Body), MPI_BYTE, MPI_COMM_WORLD);

        for (int i = 0; i < local_n; i++) {
            double fx=0, fy=0;
            for (int j = 0; j < NUM_BODIES; j++) {
                double dx = all[j].x - local[i].x;
                double dy = all[j].y - local[i].y;
                double dist = sqrt(dx*dx + dy*dy);
                if (dist > 0) {
                    double f = G * local[i].mass * all[j].mass / (dist*dist);
                    fx += f * dx / dist;
                    fy += f * dy / dist;
                }
            }
            local[i].vx += fx / local[i].mass * DT;
            local[i].vy += fy / local[i].mass * DT;
            local[i].x += local[i].vx * DT;
            local[i].y += local[i].vy * DT;
        }
    }

    MPI_Finalize();
    return 0;
}
