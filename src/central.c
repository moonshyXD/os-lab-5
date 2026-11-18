#include "contracts.h"

#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

enum { ROOT_LIMIT = 46340, BLOCK_SIZE = 32768 };
_Static_assert(INT_MAX == 2147483647, "sieve requires 32-bit int");

float Derivative(float A, float deltaX) {
    const double point = A;
    const double step = deltaX;
    return (float)((cos(point + step) - cos(point - step)) / (2.0 * step));
}

static void sieve_base(int limit, bool composite[]) {
    for (int prime = 2; prime <= limit / prime; ++prime) {
        if (composite[prime]) {
            continue;
        }
        for (int multiple = prime * prime; multiple <= limit; multiple += prime) {
            composite[multiple] = true;
        }
    }
}

static int count_block(int64_t first, int64_t last, const bool base[]) {
    bool composite[BLOCK_SIZE] = {false};
    for (int64_t prime = 2; prime * prime <= last; ++prime) {
        if (base[prime]) {
            continue;
        }
        int64_t multiple = ((first + prime - 1) / prime) * prime;
        if (multiple < prime * prime) {
            multiple = prime * prime;
        }
        for (; multiple <= last; multiple += prime) {
            composite[multiple - first] = true;
        }
    }

    int count = 0;
    for (int64_t number = first; number <= last; ++number) {
        count += !composite[number - first];
    }
    return count;
}

int PrimeCount(int A, int B) {
    bool base[ROOT_LIMIT + 1] = {false};
    sieve_base((int)sqrt(B), base);

    int count = 0;
    for (int64_t first = A > 2 ? A : 2; first <= B; first += BLOCK_SIZE) {
        const int64_t block_end = first + BLOCK_SIZE - 1;
        const int64_t last = block_end < B ? block_end : B;
        count += count_block(first, last, base);
    }
    return count;
}
