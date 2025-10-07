/*
 * Author: Hilal Mufti <hilalmufti1@gmail.com>
 * Date: 10/06/2025
 *
 * babel speaks all languages
 *
 * compile and run with: clang -O0 -Wall -Wconversion -Werror --std=c99 -g -o babel babel.c && ./babel
 */
#include <assert.h>  /* for assert */
#include <stdbool.h> /* for bool */
#include <stdio.h>   /* for EOF, getline, printf, putchar, size_t, ssize_t, stdin */
#include <stdlib.h>  /* for free */
#include <string.h>  /* for strcmp */
#include "babel.h"

unsigned int mygetline(char *line, unsigned int lim) {
    int c;
    unsigned int i = 0;

    while (i < lim - 1 && (c = getchar()) != EOF && c != '\n')
        line[i++] = c;
    line[i] = '\0';

    if (i == lim - 1)
        fprintf(stderr, "babel: mygetdelim: reached capacity\n");

    return i;
}

void prologue() {
    printf("babel version %s %s. babel speaks all languages\n", BABEL_VERSION, BABEL_DATE);
    printf("\n%s\n", BABEL_LOGO);
}

/* TODO: this breaks on EOF */
void interact() {
    char line[BUFSIZE] = {0};

    while (1) {
        printf("babel> ");
        mygetline(line, BUFSIZE);
        printf("%s\n", line);
    }
}

int main(int argc, char *argv[]) {
    bool quiet_mode = false;
    bool filter_mode = false;
    for (int i = 0; i < argc; i++) {
        quiet_mode = quiet_mode || (strcmp(argv[i], "-q") == 0);
        filter_mode = filter_mode || (strcmp(argv[i], "-f") == 0);
    }

    /* char* s = "foo baz"; */
    char *s, *tofree, *tok;
    /* why does strsep break if s is on the stack? */
    tofree = s = strdup("foo baz"); /* TODO: free this */
    printf("%s\n", s);

    tok = strsep(&s, " ");
    printf("\ns = %s, tok = %s\n", s, tok);

    /*
     * s -> ['f', 'o', 'o', ' ', 'b', 'a', 'z', '\0']
     * &s -> s -> ['f', 'o', 'o', ' ', 'b', 'a', 'z', '\0']
     */

    tok = strsep(&s, " ");
    printf("\ns = %s, tok = %s\n", s, tok);

    /* tok = strsep(&s, " "); */
    /* printf("\ns = %s, tok = %s\n", s, tok); */


    if (!quiet_mode)
        prologue();

    if (filter_mode) {
        char line[BUFSIZE] = {0};

        while (mygetline(line, BUFSIZE) > 0) {
            int x, y;

            for (int i = 0; line[i] != '\0'; i++) {
                if (line[i] == ' ')
                    printf("X");
                else
                    printf("%c", line[i]);
            }
            printf("\n");

            /* printf("%s\n", line); */
        }
    } else
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
 *
 *   - tensor/ml (extra), (target code from tensor programs)
 *   - verilog? (extra)
 *   - quantum (extra)
 *   - cryptography (extra)
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
