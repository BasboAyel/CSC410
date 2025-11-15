#include <stdio.h>

#define N 100000 // intervals

double f(double x) {
    return 4.0 / (1.0 + x * x); // Function to integrate
}

double trapezoidalRule() 
{
    // complete this
    double a = 0.0;  // Lower limit
    double b = 1.0;  // Upper limit
    double h = (b - a) / N;  // Step size
    double sum = f(a) + f(b);  // First and last terms
    
    for (int i = 1; i < N; i++) {
        double x = a + i * h;
        sum += 2.0 * f(x);
    }
    
    return (h / 2.0) * sum;
}

int main() {
    double pi = trapezoidalRule();
    printf("Estimated value of π: %f\n", pi);
    return 0;
}
