/*
 * Author: Hilal Mufti <hilalmufti1@gmail.com>
 * Date: 10/06/2025
 *
 * babel speaks all languages
 *
 * compile and run with: clang -O0 -Wall -Wconversion -Werror --std=c99 -g -o babel babel.c && ./babel
 */
#include <stdbool.h> /* for bool */
#include <stdio.h>   /* for EOF, getline, printf, putchar, size_t, ssize_t, stdin */
#include <stdlib.h>  /* for free */
#include <string.h>  /* for strcmp */
#include "babel.h"

void putline() {
    char* line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    if ((linelen = getline(&line, &linecap, stdin)) > 0)
        printf("%s\n", line);
    else {
        /* TODO: test this branch */
        fprintf(stderr, "babel: putline: error getting line\n");
        exit(1);
    }
    free(line);
}

void prologue() {
    printf("babel version %s %s. babel speaks all languages\n", BABEL_VERSION, BABEL_DATE);
    printf("\n%s\n", BABEL_LOGO);
}

void interact() {
    while (1) {
        printf("babel> ");
        putline();
    }
}

int main(int argc, char* argv[]) {
    bool quiet_mode = false;
    bool filter_mode = false;
    for (int i = 0; i < argc; i++) {
        quiet_mode = quiet_mode || (strcmp(argv[i], "-q") == 0);
        filter_mode = filter_mode || (strcmp(argv[i], "-f") == 0);
    }

    if (!quiet_mode)
        prologue();

    if (filter_mode)
        putline();
    else
        interact();
}

/*
 * TODO:
 * - maybe color or make the logo cooler?
 * - add support for piping
 * - add exiting prompt (thank you)
 *
 * - 3 language frontends:
 *   - scheme
 *   - c
 *   - synthesizer
 *   - verilog? (extra)
 *
 * - allow users to specify rewrite rules
 *
 * - 3 interfaces:
 *   - c library
 *   - interactive mode
 *   - filter mode
 *
 *   - tui mode
 *   - language model mode
 */
