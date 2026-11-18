#define _POSIX_C_SOURCE 200809L

#include "commands.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *next_token(char **rest) {
    char *start = *rest;
    while (isspace((unsigned char)*start)) {
        ++start;
    }
    char *end = start;
    while (*end != '\0' && !isspace((unsigned char)*end)) {
        ++end;
    }
    if (*end != '\0') {
        *end++ = '\0';
    }
    *rest = end;
    return start;
}

static bool parse_float(const char *token, float *value) {
    char *end;
    errno = 0;
    *value = strtof(token, &end);
    return end != token && *end == '\0' && errno == 0 && isfinite(*value);
}

static bool parse_bound(const char *token, int *value) {
    char *end;
    errno = 0;
    const long number = strtol(token, &end, 10);
    if (end == token || *end != '\0' || errno != 0 || number < 1 || number > INT_MAX) {
        return false;
    }
    *value = (int)number;
    return true;
}

static int execute(char *line, struct Contracts *contracts, int (*switch_library)(void)) {
    char *command = next_token(&line);
    if (*command == '\0') {
        return 0;
    }

    const char *first = next_token(&line);
    const char *second = next_token(&line);
    if (strcmp(command, "0") == 0 && *first == '\0' && switch_library != NULL) {
        return switch_library();
    }

    const bool two_arguments = *first != '\0' && *second != '\0' && *next_token(&line) == '\0';
    int written = 0;
    if (two_arguments && strcmp(command, "1") == 0) {
        float point;
        float step;
        if (parse_float(first, &point) && parse_float(second, &step) && step != 0.0f) {
            written = printf("%.9g\n", (double)contracts->derivative(point, step));
        }
    } else if (two_arguments && strcmp(command, "2") == 0) {
        int first_bound;
        int last_bound;
        if (parse_bound(first, &first_bound) && parse_bound(second, &last_bound) &&
            first_bound <= last_bound) {
            written = printf("%d\n", contracts->prime_count(first_bound, last_bound));
        }
    }

    if (written < 0) {
        perror("stdout");
        return -1;
    }
    if (written == 0) {
        fprintf(stderr, "Invalid command. Use: %s1 A deltaX, 2 A B\n",
                switch_library != NULL ? "0, " : "");
    }
    return 0;
}

int run_commands(struct Contracts *contracts, int (*switch_library)(void)) {
    char *line = NULL;
    size_t capacity = 0;
    int status = EXIT_SUCCESS;
    for (;;) {
        const ssize_t length = getline(&line, &capacity, stdin);
        if (length < 0) {
            if (!feof(stdin) || ferror(stdin)) {
                perror("getline");
                status = EXIT_FAILURE;
            }
            break;
        }
        if (memchr(line, '\0', (size_t)length) != NULL) {
            fputs("Invalid command: embedded NUL\n", stderr);
            continue;
        }
        if (execute(line, contracts, switch_library) != 0) {
            status = EXIT_FAILURE;
            break;
        }
        if (fflush(stdout) == EOF) {
            perror("stdout");
            status = EXIT_FAILURE;
            break;
        }
    }
    free(line);
    return status;
}
