#include "commands.h"

int main(void) {
    struct Contracts contracts = {Derivative, PrimeCount};
    return run_commands(&contracts, 0);
}
