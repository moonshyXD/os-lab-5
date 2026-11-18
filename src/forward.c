#include "contracts.h"

#include <math.h>

float Derivative(float A, float deltaX) {
    const double point = A;
    const double step = deltaX;
    return (float)((cos(point + step) - cos(point)) / step);
}

int PrimeCount(int A, int B) {
    int count = 0;
    for (int number = A > 2 ? A : 2; number <= B; ++number) {
        int divisor = 2;
        while (divisor < number && number % divisor != 0) {
            ++divisor;
        }
        count += divisor == number;
        if (number == B) {
            break;
        }
    }
    return count;
}
