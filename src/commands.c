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

static int execute(const char *command, char *arguments, struct Contracts *contracts,
                   int (*switch_library)(void), bool interactive) {
    if (*command == '\0') {
        return 0;
    }

    const char *first = next_token(&arguments);
    const char *second = next_token(&arguments);
    if (strcmp(command, "0") == 0 && *first == '\0' && switch_library != NULL) {
        return switch_library();
    }

    const bool two_arguments = *first != '\0' && *second != '\0' && *next_token(&arguments) == '\0';
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

enum InputStatus {
    INPUT_READY,
    INPUT_SKIP,
    INPUT_END,
    INPUT_ERROR
};

struct CommandInput {
    char *line;
    size_t capacity;
    const char *command;
    char *arguments;
};

static void show_menu(bool can_switch) {
    printf("Лабораторная работа №5. Вариант 9\n"
           "Текущая реализация: правая разность и перебор делителей.\n"
           "1 — производная cos(x)\n"
           "2 — количество простых чисел на отрезке\n");
    if (can_switch) {
        printf("0 — переключить реализацию\n");
    }
    printf("q — выход\n");
}

static enum InputStatus read_line(struct CommandInput *input, const char *prompt) {
    if (prompt != NULL && (fputs(prompt, stdout) == EOF || fflush(stdout) == EOF)) {
        perror("stdout");
        return INPUT_ERROR;
    }

    const ssize_t length = getline(&input->line, &input->capacity, stdin);
    if (length < 0) {
        if (feof(stdin) && !ferror(stdin)) {
            return INPUT_END;
        }
        perror("getline");
        return INPUT_ERROR;
    }

    if (memchr(input->line, '\0', (size_t)length) != NULL) {
        return INPUT_SKIP;
    }
    return INPUT_READY;
}

static enum InputStatus read_command(struct CommandInput *input, bool interactive) {
    enum InputStatus status = read_line(input, interactive ? "\nКоманда: " : NULL);
    if (status == INPUT_SKIP) {
        fputs("Invalid command: embedded NUL\n", stderr);
    }
    if (status != INPUT_READY) {
        return status;
    }
    if (interactive && (strcmp(input->line, "q\n") == 0 || strcmp(input->line, "q") == 0)) {
        return INPUT_END;
    }

    const char *prompt = NULL;
    if (interactive && strcmp(input->line, "1\n") == 0) {
        input->command = "1";
        prompt = "Введите точку и ненулевой шаг через пробел (например, 0 0.5): ";
    } else if (interactive && strcmp(input->line, "2\n") == 0) {
        input->command = "2";
        prompt = "Введите границы отрезка через пробел (например, 1 100): ";
    }

    if (prompt != NULL) {
        status = read_line(input, prompt);
        if (status == INPUT_SKIP) {
            fputs("Неверный ввод: обнаружен нулевой байт.\n", stderr);
        }
        input->arguments = input->line;
    } else {
        input->arguments = input->line;
        input->command = next_token(&input->arguments);
    }
    return status;
}

int run_commands(struct Contracts *contracts, int (*switch_library)(void)) {
    struct CommandInput input = {0};
    int status = EXIT_SUCCESS;
    const bool interactive = isatty(STDIN_FILENO);
    if (interactive) {
        show_menu(switch_library != NULL);
    }

    for (;;) {
        const enum InputStatus read_status = read_command(&input, interactive);
        if (read_status == INPUT_SKIP) {
            continue;
        }
        if (read_status == INPUT_END) {
            break;
        }
        if (read_status == INPUT_ERROR ||
            execute(input.command, input.arguments, contracts, switch_library, interactive) != 0) {
            status = EXIT_FAILURE;
            break;
        }
        if (fflush(stdout) == EOF) {
            perror("stdout");
            status = EXIT_FAILURE;
            break;
        }
    }

    free(input.line);
    return status;
}
