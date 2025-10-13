/*
 * Author: Hilal Mufti <hilalmufti1@gmail.com>
 * Date: 10/06/2025
 *
 * babel speaks all languages
 *
 * compile and run with: clang -O0 -Wall -Wconversion -Werror --std=c99 -g -o babel babel.c && ./babel
 * cat data/fib.txt | make filter-leaks
 */
#include <assert.h>  /* for assert */
#include <stdbool.h> /* for bool */
#include <stdio.h>   /* for EOF, getline, printf, putchar, size_t, ssize_t, stdin */
#include <stdlib.h>  /* for free */
#include <string.h>  /* for strcmp */
#include <time.h>
#include "babel.h"

Arena barena; /* babel arena */

void ainit(Arena *a) {
    a->buf = calloc(MEMCAP, 1);
    a->len = 0;
    a->cap = MEMCAP;
}

void adeinit(Arena *a) {
    free(a->buf);
    a->buf = NULL;
    a->len = 0;
    a->cap = 0;
}

/* TODO: This is unaligned */
void *aalloc(Arena *a, unsigned int nbytes) {
    void *ptr;

    assert(a->len + nbytes < a->cap);

    ptr = a->buf + a->len;
    a->len += nbytes;
    return ptr;
}

/* TODO: this is super unoptimized */
/* TODO: test this shit!!! */
void *arealloc(Arena *a, void *oldptr, unsigned int nbytes) {
    void *newptr;

    assert(a->len + nbytes < a->cap);

    newptr = aalloc(a, nbytes);

    char *op = oldptr;
    char *np = newptr;
    for (int i = 0; i < nbytes; i++) /* TODO: add an amemcpy */
        np[i] = op[i];

    return newptr;
}

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

/* TODO: BinOp and Expr are tightly coupled right now. Fix later? */
char *bopshow(BinOp *bop) {
    char buf[BUFSIZE] = {0};

    char *s1;
    char *s2;
    switch (bop->tag) {
        case BINOP_ADD:
            s1 = eshow(bop->e1);
            s2 = eshow(bop->e2);
            snprintf(buf, BUFSIZE, "BinOp (Add (%s) (%s))", s1, s2);
            free(s1);
            free(s2);
            return strndup(buf, BUFSIZE);
        case BINOP_MUL:
            s1 = eshow(bop->e1);
            s2 = eshow(bop->e2);
            snprintf(buf, BUFSIZE, "BinOp (Mul (%s) (%s))", s1, s2);
            free(s1);
            free(s2);
            return strndup(buf, BUFSIZE);
        default: assert(false);
    }
}

char *eshow(Expr *e) {
    char buf[BUFSIZE] = {0}; /* TODO: This is definitely too big */

    char *s;
    switch (e->tag) {
        case EXPR_BINOP: /* TODO: memory leak here */
            s = bopshow(e->as.binop);
            snprintf(buf, BUFSIZE, "Expr (%s)", s);
            free(s);
            return strndup(buf, BUFSIZE);
        case EXPR_NUM:
            snprintf(buf, BUFSIZE, "Expr (Num %d)", e->as.num);
            return strndup(buf, BUFSIZE); /* TODO: off-by-1 error? */
        default: assert(false);
    }
}

/* TODO: This allocates */
/* TODO: Should I split an allocation function separately? */
void psinit(Programs *ps, Arena *a) {
    /* ps->buf = calloc(MAXPROGRAMS, sizeof(PROGRAM_T)); */
    ps->buf = aalloc(a, MAXPROGRAMS * sizeof(PROGRAM_T));
    ps->cap = MAXPROGRAMS;
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
        printf("ps->len = %d, ps->cap = %d\n", ps->len, ps->cap);
        assert(false);
        ps->buf = realloc(ps->buf, sizeof(*(ps->buf)) * 2 * ps->cap);
        ps->cap = 2 * ps->cap;
    }
    ps->buf[ps->len++] = p;
}

Expr *pslookup(Programs *ps, unsigned int i) {
    return ps->buf + i;
}

void psgrow(Programs *ps, Arena *a) {
    unsigned int len = ps->len;
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < len; j++) {
            /* TODO: it's not safe to point to yourself because of reallocs */
            Expr *e1 = pslookup(ps, i);
            Expr *e2 = pslookup(ps, j);

            BinOp add = ADD_BINOP(e1, e2);
            /* BinOp *addcpy = calloc(1, sizeof(add)); /\* TODO: leak *\/ */
            BinOp *addcpy = aalloc(a, sizeof(add));
            memmove(addcpy, &add, sizeof(add));
            psput(ps, BINOP_EXPR(addcpy));

            BinOp mul = MUL_BINOP(e1, e2);
            /* BinOp *mulcpy = calloc(1, sizeof(mul)); */
            BinOp *mulcpy = aalloc(a, sizeof(mul));
            memmove(mulcpy, &mul, sizeof(mul));
            psput(ps, BINOP_EXPR(mulcpy));
        }
    }
}

void pselim(Programs *ps, Samples *ss) {
    /* TODO */
}

void ssinit(Samples *ss, Arena *a) {
    /* ss->xs = calloc(ARRCAP, sizeof(int)); */
    /* ss->ys = calloc(ARRCAP, sizeof(int)); */
    ss->xs = aalloc(a, ARRCAP * sizeof(int));
    ss->ys = aalloc(a, ARRCAP * sizeof(int));
    ss->cap = ARRCAP;
    ss->len = 0;
}

void ssdeinit(Samples *ss) {
    free(ss->xs);
    free(ss->ys);
    ss->cap = 0;
    ss->len = 0;
}

void ssput(Samples *ss, int x, int y, Arena *a) {
    if (ss->len == ss->cap) {
        /* ss->xs = realloc(ss->xs, sizeof(*(ss->xs)) * 2 * ss->cap); */
        /* ss->ys = realloc(ss->ys, sizeof(*(ss->ys)) * 2 * ss->cap); */
        ss->xs = arealloc(a, ss->xs, sizeof(*(ss->xs)) * 2 * ss->cap);
        ss->ys = arealloc(a, ss->xs, sizeof(*(ss->ys)) * 2 * ss->cap);
        ss->cap = 2 * ss->cap;
    }
    ss->xs[ss->len] = x;
    ss->ys[ss->len] = y;
    ss->len += 1;
}

Programs sssynthesize(Samples *ss, unsigned int depth, Arena *a) {
    Programs ps = {0};

    /* Add terminals to program list */
    psinit(&ps, a);
    for (int i = -10; i <= 10; i++)
        psput(&ps, NUM_EXPR(i));

    for (int i = 0; i < depth; i++)
        psgrow(&ps, a);

    return ps;
}

int main(int argc, char *argv[]) {
    bool quiet_mode = false;
    bool filter_mode = false;
    for (int i = 0; i < argc; i++) {
        quiet_mode = quiet_mode || (strcmp(argv[i], "-q") == 0);
        filter_mode = filter_mode || (strcmp(argv[i], "-f") == 0);
    }

    if (!quiet_mode)
        prologue();

    ainit(&barena);

    Samples ss = {0};
    ssinit(&ss, &barena);
    if (filter_mode) {
        char line[BUFSIZE] = {0};

        while (mygetline(line, BUFSIZE) > 0) {
            char *tok;
            int x, y;

            /* TODO: extend this to > 2 tokens */
            char *s = line;
            x = atoi((tok = strsep(&s, " ")));
            y = atoi((tok = strsep(&s, " ")));
            ssput(&ss, x, y, &barena);
        }
    } else
        interact();

    printf("\nSAMPLES\n");
    printf("n = %d\n", ss.len);
    for (int i = 0; i < ss.len; i++)
        printf("%d %d\n", ss.xs[i], ss.ys[i]);

    clock_t tic = clock();

    Programs ps = sssynthesize(&ss, 2, &barena);

    clock_t toc = clock();
    unsigned int dt = (((double)(toc - tic) / CLOCKS_PER_SEC) * 1000000000); /* time in ns */

    printf("\nPROGRAMS\n");
    printf("n = %d\n", ps.len);
    printf("dt = %d\n", dt);
    printf("\n");
    for (int i = 0; i < 10; i++) {
        Expr *e = pslookup(&ps, i);
        char *s = eshow(e);
        printf("%s\n", s);
        free(s);
    }
    if (ps.len > 10)
        printf("...\n");

    adeinit(&barena);
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
