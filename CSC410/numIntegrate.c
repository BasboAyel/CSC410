#include <stdio.h>

#define N 100000 // intervals

double f(double x) {
    return 4.0 / (1.0 + x * x); // Function to integrate
}

double trapezoidalRule() 
{
    // complete this
     double a = 0.0, b = 1.0;
    double h = (b - a) / (double)N;
    double sum = 0.5 * (f(a) + f(b));
    for (int i = 1; i < N; i++) {
        double x = a + i * h;
        sum += f(x);
    }
    return sum * h;
}

int main() {
    double pi = trapezoidalRule();
    printf("Estimated value of π: %f\n", pi);
    return 0;
}
