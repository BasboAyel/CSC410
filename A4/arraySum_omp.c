#include <stdio.h>
#include <omp.h>

#define SIZE 100000000  // large size for timing

int main() {
    static int arr[SIZE];
    for (int i = 0; i < SIZE; i++) arr[i] = i + 1;

    double start = omp_get_wtime();
    long long sum = 0;

    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i];
    }

    double end = omp_get_wtime();
    printf("Total Sum: %lld\n", sum);
    printf("Time: %f seconds\n", end - start);
    return 0;
}
