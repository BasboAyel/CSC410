#include <stdio.h>
#include <omp.h>

#define N 100000000

double f(double x) {
    return 4.0 / (1.0 + x * x);
}

int main() {
    double a = 0.0, b = 1.0;
    double h = (b - a) / (double)N;
    double sum = 0.5 * (f(a) + f(b));

    double start = omp_get_wtime();

    #pragma omp parallel for reduction(+:sum)
    for (int i = 1; i < N; i++) {
        double x = a + i * h;
        sum += f(x);
    }

    double pi = sum * h;
    double end = omp_get_wtime();

    printf("Estimated value of π: %.12f\n", pi);
    printf("Time: %f seconds\n", end - start);
    return 0;
}
