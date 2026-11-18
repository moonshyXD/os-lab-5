#include "commands.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

static const char *const library_paths[] = {
    "./build/libforward.so",
    "./build/libcentral.so"
};

static void *library;
static unsigned active_index;
static struct Contracts contracts;

static int close_library(void *handle)
{
    if (dlclose(handle) != 0) {
        fprintf(stderr, "dlclose: %s\n", dlerror());
        return -1;
    }
    return 0;
}

static void *find_symbol(void *handle, const char *name)
{
    dlerror();
    void *symbol = dlsym(handle, name);
    const char *error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "dlsym %s: %s\n", name, error);
        return NULL;
    }
    if (symbol == NULL) {
        fprintf(stderr, "dlsym %s: null function address\n", name);
    }
    return symbol;
}

static int load_library(unsigned index)
{
    void *candidate = dlopen(library_paths[index], RTLD_NOW | RTLD_LOCAL);
    if (candidate == NULL) {
        fprintf(stderr, "dlopen %s: %s\n", library_paths[index], dlerror());
        return 1;
    }

    struct Contracts next;
    next.derivative = (float (*)(float, float))find_symbol(candidate, "Derivative");
    next.prime_count = (int (*)(int, int))find_symbol(candidate, "PrimeCount");
    if (next.derivative == NULL || next.prime_count == NULL) {
        return close_library(candidate) == 0 ? 1 : -1;
    }

    if (library != NULL && close_library(library) != 0) {
        close_library(candidate);
        return -1;
    }
    library = candidate;
    contracts = next;
    active_index = index;
    return 0;
}

static int switch_library(void)
{
    const int result = load_library(1U - active_index);
    if (result > 0) {
        fprintf(stderr, "Active library retained: %s\n", library_paths[active_index]);
        return 0;
    }
    if (result < 0) {
        return -1;
    }
    if (printf("Active library: %s\n", library_paths[active_index]) < 0) {
        perror("stdout");
        return -1;
    }
    return 0;
}

int main(void)
{
    if (load_library(0) != 0) {
        return EXIT_FAILURE;
    }
    const int status = run_commands(&contracts, switch_library);
    if (close_library(library) != 0) {
        return EXIT_FAILURE;
    }
    return status;
}
