/*
 * Author: Hilal Mufti <hilalmufti1@gmail.com>
 * Date: 10/06/2025
 *
 * babel speaks all languages
 *
 * compile and run with: clang -O0 -Wall -Wconversion -Werror --std=c99 -g -o babel babel.c && ./babel
 */
#include <stdio.h> /* for EOF, getchar, printf, putchar */
#include <string.h> /* for strcmp */
#include "babel.h"

void prologue() {
    printf("babel version %s %s. babel speaks every language\n", BABEL_VERSION, BABEL_DATE);
    printf("\n");
    printf("%s\n", BABEL_LOGO);
}

void interact() {
    int c;

    while (1) {
        printf("babel> ");
        while ((c = getchar()) != '\n')
            putchar(c);
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1 || !((argc >= 2 && strcmp(argv[1], "-q") == 0) || (argc >= 3 && strcmp(argv[1], "-q") == 0)))
        prologue();
    if (argc == 1 || (argc >= 2 && strcmp(argv[1], "-f") != 0) || (argc >= 3 && strcmp(argv[1], "-f") != 0))
        interact();

    /* int c, nl; */

    /* nl = 0; */
    /* while ((c = getchar()) != EOF) */
    /*     if (c == '\n') */
    /*         ++nl; */
    /* printf("%d\n", nl); */

}

/*
 * TODO:
 * - maybe color or make the logo cooler?
 * - add support for piping
 * - add exiting prompt (thank you)
 *
 * - 3 interfaces:
 *   - c library
 *   - interactive mode
 *   - filter mode
 */
