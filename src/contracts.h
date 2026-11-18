#ifndef CONTRACTS_H
#define CONTRACTS_H

float Derivative(float A, float deltaX);
int PrimeCount(int A, int B);

struct Contracts {
    float (*derivative)(float, float);
    int (*prime_count)(int, int);
};

#endif
