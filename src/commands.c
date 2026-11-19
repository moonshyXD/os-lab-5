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
#include <unistd.h>

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

static int execute(char *line, struct Contracts *contracts, int (*switch_library)(void), bool interactive) {
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
            written = printf(interactive ? "Производная: %.9g\n" : "%.9g\n",
                             (double)contracts->derivative(point, step));
        }
    } else if (two_arguments && strcmp(command, "2") == 0) {
        int first_bound;
        int last_bound;
        if (parse_bound(first, &first_bound) && parse_bound(second, &last_bound) &&
            first_bound <= last_bound) {
            written = printf(interactive ? "Количество простых чисел: %d\n" : "%d\n",
                             contracts->prime_count(first_bound, last_bound));
        }
    }

    if (written < 0) {
        perror("stdout");
        return -1;
    }
    if (written == 0) {
        if (interactive) {
            fputs("Неверный ввод. Введите два числа через пробел и попробуйте снова.\n", stderr);
        } else {
            fprintf(stderr, "Invalid command. Use: %s1 A deltaX, 2 A B\n",
                    switch_library != NULL ? "0, " : "");
        }
    }
    return 0;
}

int run_commands(struct Contracts *contracts, int (*switch_library)(void)) {
    char *line = NULL;
    size_t capacity = 0;
    int status = EXIT_SUCCESS;
    const bool interactive = isatty(STDIN_FILENO);
    if (interactive) {
        printf("Лабораторная работа №5. Вариант 9\n"
               "Текущая реализация: правая разность и перебор делителей.\n"
               "1 — производная cos(x)\n"
               "2 — количество простых чисел на отрезке\n");
        if (switch_library != NULL) {
            printf("0 — переключить реализацию\n");
        }
        printf("q — выход\n");
    }
    for (;;) {
        if (interactive && (printf("\nКоманда: ") < 0 || fflush(stdout) == EOF)) {
            perror("stdout");
            status = EXIT_FAILURE;
            break;
        }
        ssize_t length = getline(&line, &capacity, stdin);
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
        if (interactive && (strcmp(line, "q\n") == 0 || strcmp(line, "q") == 0)) {
            break;
        }
        if (interactive && (strcmp(line, "1\n") == 0 || strcmp(line, "2\n") == 0)) {
            const char command = line[0];
            const char *prompt = command == '1'
                ? "Введите точку и ненулевой шаг через пробел (например, 0 0.5): "
                : "Введите границы отрезка через пробел (например, 1 100): ";
            if (printf("%s", prompt) < 0 || fflush(stdout) == EOF) {
                perror("stdout");
                status = EXIT_FAILURE;
                break;
            }
            length = getline(&line, &capacity, stdin);
            if (length < 0) {
                if (!feof(stdin) || ferror(stdin)) {
                    perror("getline");
                    status = EXIT_FAILURE;
                }
                break;
            }
            if (memchr(line, '\0', (size_t)length) != NULL) {
                fputs("Неверный ввод: обнаружен нулевой байт.\n", stderr);
                continue;
            }
            char *input = malloc((size_t)length + 3);
            if (input == NULL) {
                perror("malloc");
                status = EXIT_FAILURE;
                break;
            }
            input[0] = command;
            input[1] = ' ';
            memcpy(input + 2, line, (size_t)length + 1);
            int result = execute(input, contracts, switch_library, true);
            free(input);
            if (result != 0) {
                status = EXIT_FAILURE;
                break;
            }
        } else if (execute(line, contracts, switch_library, interactive) != 0) {
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
