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
        fprintf(stderr, "babel: mygetline: reached capacity\n");

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

void sampleinit(Sample *sam, int x, int y) {
    sam->x = x;
    sam->y = y;
}

char *bopshow(BinOp *bop) {
    char buf[BUFSIZE] = {0};

    switch (bop->tag) {
        case BINOP_ADD:
            snprintf(buf, BUFSIZE, "BinOp (Add (%s) (%s))", eshow(bop->e1), eshow(bop->e2));
            return strndup(buf, BUFSIZE);
        default: assert(false);
    }
}

char *eshow(Expr *e) {
    char buf[BUFSIZE] = {0}; /* TODO: This is definitely too big */

    switch (e->tag) {
        case EXPR_BINOP: /* TODO: memory leak here */
            snprintf(buf, BUFSIZE, "Expr (%s)", bopshow(e->as.binop));
            return strndup(buf, BUFSIZE);
        case EXPR_NUM:
            snprintf(buf, BUFSIZE, "Expr (Num %d)", e->as.num);
            return strndup(buf, BUFSIZE); /* TODO: off-by-1 error? */
        default: assert(false);
    }
}

/* TODO: This allocates */
/* TODO: Should I split an allocation function separately? */
void psinit(Programs *ps) {
    ps->buf = calloc(ARRCAP, sizeof(PROGRAM_T));
    ps->cap = ARRCAP;
    ps->len = 0;
}

void psdeinit(Programs *ps) {
    free(ps->buf);

    ps->buf = NULL;
    ps->cap = 0;
    ps->len = 0;
}

void psput(Programs *ps, Expr p) {
    if (ps->len == ps->cap) {
        ps->buf = realloc(ps->buf, sizeof(*(ps->buf)) * 2 * ps->cap);
        ps->cap = 2 * ps->cap;
    }
    ps->buf[ps->len++] = p;
}

Expr pslookup(Programs *ps, unsigned int i) {
    return ps->buf[i];
}

void psgrow(Programs *ps) {
    unsigned int len = ps->len;
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < len; j++) {
            BinOp bop = ADD_BINOP(ps->buf + i, ps->buf + j);

            BinOp *copy = calloc(1, sizeof(bop));
            memmove(copy, &bop, sizeof(bop));

            psput(ps, BINOP_EXPR(copy));
        }
    }
}

void pselim(Programs *ps) {
    /* TODO */
}

Programs synthesize(Sample *samples, unsigned int nsamples) {
    Programs ps = {0};

    /* Add terminals to program list */
    psinit(&ps);
    for (int i = -1; i <= 1; i++)
        psput(&ps, NUM_EXPR(i));

    psgrow(&ps);

    return ps;
}

int main(int argc, char *argv[]) {
    bool quiet_mode = false;
    bool filter_mode = false;
    for (int i = 0; i < argc; i++) {
        quiet_mode = quiet_mode || (strcmp(argv[i], "-q") == 0);
        filter_mode = filter_mode || (strcmp(argv[i], "-f") == 0);
    }

    Expr e2 = NUM_EXPR(2);
    Expr e3 = NUM_EXPR(3);
    Expr e5 = NUM_EXPR(5);

    BinOp add23 = ADD_BINOP(&e2, &e3);

    /* Expr eadd = BINOP_EXPR(&add23); */
    Expr eadd = BINOP_EXPR(&ADD_BINOP(&NUM_EXPR(2), &NUM_EXPR(3)));

    char *s2 = eshow(&e2);
    char *sadd = eshow(&eadd);
    printf("e2   = %s\n", s2);
    printf("eadd = %s\n", sadd);
    free(s2);
    free(sadd); /* TODO: This doesn't free subexpressions */

    if (!quiet_mode)
        prologue();

    Sample samples[DATASIZE] = {0};
    unsigned int nsamples = 0;
    if (filter_mode) {
        char line[BUFSIZE] = {0};

        while (mygetline(line, BUFSIZE) > 0) {
            /* TODO: This code is too dense */
            char *tok;
            char *s = line;
            int nums[MAXTOKS] = {0};
            unsigned int nnums = 0;
            while ((tok = strsep(&s, " ")) != NULL) /* TODO: Why can't I pass line here? */
                nums[nnums++] = atoi(tok);

            sampleinit(samples + (nsamples++), nums[0], nums[1]);
        }
    } else
        interact();

    printf("\nSAMPLES\n");
    printf("len(samples) = %d\n", nsamples);
    for (int i = 0; i < nsamples; i++)
        printf("%d %d\n", samples[i].x, samples[i].y);

    Programs ps = synthesize(samples, nsamples);
    printf("\nPROGRAMS\n");
    for (int i = 0; i < ps.len; i++) {
        Expr e = pslookup(&ps, i);
        printf("%s\n", eshow(&e));
    }

    psdeinit(&ps);
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
