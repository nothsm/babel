/*
 * Author: Hilal Mufti <hilalmufti1@gmail.com>
 * Date: 10/06/2025
 *
 * babel speaks all languages
 *
 * factorial demo: cat data/factorial.txt | make filter
 * sum demo: cat data/sum.txt | make filter
 */
#include <assert.h>  /* for assert */
#include <stdbool.h> /* for bool */
#include <stdio.h>   /* for EOF, getline, printf, putchar, size_t, ssize_t, stdin */
#include <stdlib.h>  /* for free */
#include <string.h>  /* for strcmp */
#include <time.h>
#include <stdalign.h> /* TODO: remove */
#include <sys/mman.h>
#include "babel.h"

Arena barena; /* babel arena */

void ainit(Arena *a) {
    a->buf = calloc(MEMCAP, 1);
    /* a->buf = mmap(NULL, MEMCAP, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0); */
    a->len = 0;
    a->cap = MEMCAP;
}

/*
 * - Rec needs parent?
 *
 * Let Input = 3.
 *
 * (Add (Rec Input) 1)
 * (Add (Add (Rec Input) 1))
 * (Add (Add (Add (Rec Input) 1) 1) 1)
 * (Add (Add (Add 0 1) 1) 1)
 *
 * (Add (Rec Input) 1)
 * 1 + (Add (Rec (Input - 1)) 1)
 *
 * Let x = (Add (Rec (Add Input (Num -1))) (Num 1))
 *
 * 1) with context
 * (eeval (Add (Rec (Add Input (Num -1)))
 *             (Num 1))
 *        NULL
 *        3)
 * { eval Add args}
 * = (+ (eeval (Rec (Add Input (Num -1))) x 3)
 *      (eeval (Num 1) x 3))
 * { eval Rec }
 * = (+ (eeval x
 *             NULL
 *             (eeval (Add Input (Num -1)) x 3))
 *      (eeval (Num 1) x 3))
 * { eval Add, Input, Num }
 * = (+ (eeval x NULL 2)
 *      (eeval (Num 1) x 3))
 * { substitute x }
 * = (+  (eeval (Add (Rec (Add Input (Num -1)))
 *                   (Num 1))
 *              NULL
 *              2)
 *       (eeval (Num 1) x 3))
 * { eval Add args }
 * = (+  (+ (eeval (Rec (Add Input (Num -1))) x 2)
 *          (eeval (Num 1) x 2))
 *       (eeval (Num 1) x 3))
 * { eval Rec }
 * = (+  (+ (eeval x
 *                 NULL
 *                 (eeval (Add Input (Num -1)) x 2))
 *          (eeval (Num 1) x 2))
 *       (eeval (Num 1) x 3))
 * { eval Add, Input, Num }
 * = (+  (+ (eeval x NULL 1)
 *          (eeval (Num 1) x 2))
 *       (eeval (Num 1) x 3))
 * { substitute x }
 * = (+  (+ (eeval (Add (Rec (Add Input (Num -1)))
 *                 (Num 1))
 *                 NULL
 *                 1)
 *          (eeval (Num 1) x 2))
 *       (eeval (Num 1) x 3))
 * { eval Add args }
 * = (+ (+ (+ (eeval (Rec (Add Input (Num -1))) x 1)
 *            (eeval (Num 1) x 1))
 *         (eeval (Num 1) x 2))
 *      (eeval (Num 1) x 3))
 * { eval Rec }
 * = (+ (+ (+ (eeval x
 *                   NULL
 *                   (eeval (Add Input (Num -1)) x 1))
 *            (eeval (Num 1) x 1))
 *         (eeval (Num 1) x 2))
 *      (eeval (Num 1) x 3))
 * { eval Add, Input, Num }
 * = (+ (+ (+ (eeval x NULL 0)
 *            (eeval (Num 1) x 1))
 *         (eeval (Num 1) x 2))
 *      (eeval (Num 1) x 3))
 * { substitute x }
 * = (+ (+ (+ (eeval (Add (Rec (Add Input (Num -1)))
 *                        (Num 1))
 *                   NULL
 *                   0)
 *            (eeval (Num 1) x 1))
 *         (eeval (Num 1) x 2))
 *      (eeval (Num 1) x 3))
 * { eval Add args }
 * = (+ (+ (+ (+ (eeval (Rec (Add Input (Num -1))) x 0)
 *               (eeval (Num 1) x 0))
 *            (eeval (Num 1) x 1))
 *         (eeval (Num 1) x 2))
 *      (eeval (Num 1) x 3))
 * { eval Rec }
 * = (+ (+ (+ (+ 0
 *               (eeval (Num 1) x 0))
 *            (eeval (Num 1) x 1))
 *         (eeval (Num 1) x 2))
 *      (eeval (Num 1) x 3))
 * { eval Add, Num }
 * = 4
 *
 * 2) without context
 * (eeval (Rec (Add Input (Num -1))) NULL 1)
 * = (eeval (Rec (Add Input (Num -1))) NULL (eeval (Add Input (Num -1)) x 1))
 * = (eeval (Rec (Add Input (Num -1))) NULL 0)
 * = 0
 *
 * (eeval (Add (Rec (Add Input (Num -1)) x) 1) 0)
 *
 * (eeval (Add (Rec (Add Input (Num -1)) x) (Num 1)) 3)
 * = (eeval (Rec (Add Input (Num -1)) x) 3) + (eeval (Num 1) 3)                                                                                              [eval0 Add]
 * = (eeval (Add (Rec (Add Input (Num -1)) x) (Num 1)) (eeval (Add Input (Num -1)) 3)) + (eeval (Num 1) 3)                                                   [eval Rec]
 * = (eeval (Add (Rec (Add Input (Num -1)) x) (Num 1)) ((eeval Input 3) + (eeval (Num -1)))) + (eeval (Num 1) 3)                                             [eval0 Add']
 * = (eeval (Add (Rec (Add Input (Num -1)) x) (Num 1)) (3 + (eeval (Num -1)))) + (eeval (Num 1) 3)                                                           [eval Input]
 * = (eeval (Add (Rec (Add Input (Num -1)) x) (Num 1)) (3 + -1)) + (eeval (Num 1) 3)                                                                         [eval Num]
 * = (eeval (Add (Rec (Add Input (Num -1)) x) (Num 1)) 2) + (eeval (Num 1) 3)                                                                                [eval1 Add']
 * = ((eeval (Rec (Add Input (Num -1)) x) 2) + (eeval (Num 1) 2)) + (eeval (Num 1) 3)                                                                        [eval0 Add'']
 * = ((eeval (Add (Rec (Add Input (Num -1)) x) (Num 1))  (eeval (Add Input (Num -1)) 2)) + (eeval (Num 1) 2)) + (eeval (Num 1) 3)                            [eval Rec]
 * = ((eeval (Add (Rec (Add Input (Num -1)) x) (Num 1))  ((eeval Input 2) + (eeval (Num -1) 2))) + (eeval (Num 1) 2)) + (eeval (Num 1) 3)                    [eval0 Add''']
 * = ((eeval (Add (Rec (Add Input (Num -1)) x) (Num 1))  (2 + (eeval (Num -1) 2))) + (eeval (Num 1) 2)) + (eeval (Num 1) 3)                                  [eval Input]
 * = ((eeval (Add (Rec (Add Input (Num -1)) x) (Num 1))  (2 + -1)) + (eeval (Num 1) 2)) + (eeval (Num 1) 3)                                                  [eval Num]
 * = ((eeval (Add (Rec (Add Input (Num -1)) x) (Num 1)) 1) + (eeval (Num 1) 2)) + (eeval (Num 1) 3)                                                          [eval1 Add''']
 * = (((eeval (Rec (Add Input (Num -1)) x) 1) + (eeval (Num 1) 1)) + (eeval (Num 1) 2)) + (eeval (Num 1) 3)                                                  [eval0 Add'''']
 * = (((eeval (Add (Rec (Add Input (Num -1)) x) (Num 1)) (eeval (Add Input (Num -1)) 1)) + (eeval (Num 1) 1)) + (eeval (Num 1) 2)) + (eeval (Num 1) 3)       [eval Rec]
 * = (((eeval (Add (Rec (Add Input (Num -1)) x) (Num 1)) 0) + (eeval (Num 1) 1)) + (eeval (Num 1) 2)) + (eeval (Num 1) 3)                                    [eval Add''''']
 * = ((((eeval (Rec (Add Input (Num -1)) x) 0) + (eeval (Num 1) 0)) + (eeval (Num 1) 1)) + (eeval (Num 1) 2)) + (eeval (Num 1) 3)                            [eval0 Add'''''']
 * = (((0 + (eeval (Num 1) 0)) + (eeval (Num 1) 1)) + (eeval (Num 1) 2)) + (eeval (Num 1) 3)                                                                 [eval Rec]
 * = (((0 + 1) + (eeval (Num 1) 1)) + (eeval (Num 1) 2)) + (eeval (Num 1) 3)                                                                                 [eval Num]
 * = ((1 + (eeval (Num 1) 1)) + (eeval (Num 1) 2)) + (eeval (Num 1) 3)                                                                                       [eval1 Add'''''']
 * = ((1 + 1) + (eeval (Num 1) 2)) + (eeval (Num 1) 3)                                                                                                       [eval Num]
 * = (2 + (eeval (Num 1) 2)) + (eeval (Num 1) 3)                                                                                                             [eval1 Add'''']
 * = (2 + 1) + (eeval (Num 1) 3)                                                                                                                             [eval Num]
 * = 3 + (eeval (Num 1) 3)                                                                                                                                   [eval1 Add'']
 * = 3 + 1                                                                                                                                                   [eval Num]
 * = 4                                                                                                                                                       [eval1 Add]
 */

void adeinit(Arena *a) {
    free(a->buf);
    a->buf = NULL;
    a->len = 0;
    a->cap = 0;
}

/* TODO: This is unaligned */
/* TODO: this has fragmentation */
void *aalloc(Arena *a, unsigned int nbytes, unsigned int align) {
    void *ptr;

    assert(a->len + nbytes < a->cap);

    /* TODO: Do this in O(1) */
    while (((unsigned long)(a->buf + a->len)) % align != 0)
        a->len += 1;

    ptr = a->buf + a->len;
    a->len += nbytes;
    return ptr;
}

/* TODO: this is super unoptimized */
/* TODO: test this shit!!! */
/* TODO: what if align changes between calls? */
void *arealloc(Arena *a, void *oldptr, unsigned int nbytes, unsigned int align) {
    void *newptr;

    assert(a->len + nbytes < a->cap);

    newptr = aalloc(a, nbytes, align);

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
/* TODO: these fine-grained allocations are killing my perf */
char *bopshow(BinOp *bop) {
    char buf[BUFSIZE] = {0};

    char *s1;
    char *s2;
    switch (bop->tag) {
        case BINOP_ADD:
            s1 = eshow(bop->e1);
            s2 = eshow(bop->e2);
            /* snprintf(buf, BUFSIZE, "BinOp Add (%s) (%s)", s1, s2); */
            snprintf(buf, BUFSIZE, "(Add %s %s)", s1, s2);
            free(s1);
            free(s2);
            return strndup(buf, BUFSIZE);
        case BINOP_MUL:
            s1 = eshow(bop->e1);
            s2 = eshow(bop->e2);
            /* snprintf(buf, BUFSIZE, "BinOp Mul (%s) (%s)", s1, s2); */
            snprintf(buf, BUFSIZE, "(Mul %s %s)", s1, s2);
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
        case EXPR_NUM:
            /* snprintf(buf, BUFSIZE, "Expr (Num %d)", e->as.num); */
            snprintf(buf, BUFSIZE, "(Num %d)", e->as.num);
            return strndup(buf, BUFSIZE); /* TODO: off-by-1 error? */
        case EXPR_INPUT:
            /* snprintf(buf, BUFSIZE, "Expr Input"); */
            snprintf(buf, BUFSIZE, "Input");
            return strndup(buf, BUFSIZE);
        case EXPR_REC:
            s = eshow(e->as.rec);
            /* snprintf(buf, BUFSIZE, "Expr (Rec (%s))", s); */
            snprintf(buf, BUFSIZE, "(Rec %s)", s); /* TODO: show parent */
            free(s);
            return strndup(buf, BUFSIZE);
        case EXPR_BINOP: /* TODO: memory leak here */
            s = bopshow(e->as.binop);
            /* snprintf(buf, BUFSIZE, "Expr (%s)", s); */
            snprintf(buf, BUFSIZE, "%s", s);
            free(s);
            return strndup(buf, BUFSIZE);
        default: assert(false);
    }
}

/* TODO: add depth/recursion limit */
/* TODO: isn't this just CPS/coroutines? */
int eeval(Expr *e, Expr *context, unsigned int depth, int input) {
    int x, y;

    if (depth >= RECLIM)
        return 0;

    switch (e->tag) {
        case EXPR_NUM:
            return e->as.num;
        case EXPR_INPUT:
            return input;
        case EXPR_REC:
            if (input <= 1) {
                if (context == NULL)
                    return 0;
                else if (context != NULL && context->tag == EXPR_BINOP && context->as.binop->tag == BINOP_ADD)
                    return 0;
                else if (context != NULL && context->tag == EXPR_BINOP && context->as.binop->tag == BINOP_MUL)
                    return 1;
                else
                    return 0;
            }
            else if (context == NULL)
                return eeval(e, NULL, depth + 1, eeval(e->as.rec, e, depth + 1, input));
            else
                return eeval(context, NULL, depth + 1, eeval(e->as.rec, e, depth + 1, input)); /* TODO: be able to "remember" the context's context */
        case EXPR_BINOP:
            switch (e->as.binop->tag) {
                case BINOP_ADD:
                    x = eeval(e->as.binop->e1, e, depth + 1, input);
                    y = eeval(e->as.binop->e2, e, depth + 1, input);
                    return x + y;
                case BINOP_MUL:
                    x = eeval(e->as.binop->e1, e, depth + 1, input);
                    y = eeval(e->as.binop->e2, e, depth + 1, input);
                    return x * y;
                default: assert(false);
            }
        default: assert(false);
    }
}

/* TODO: This allocates */
/* TODO: Should I split an allocation function separately? */
void psinit(Programs *ps, Arena *a) {
    /* ps->buf = calloc(MAXPROGRAMS, sizeof(PROGRAM_T)); */
    ps->buf = aalloc(a, MAXPROGRAMS * sizeof(PROGRAM_T), alignof(PROGRAM_T));
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

    /* TODO: add this back */
    for (int i = 0; i < len; i++) {
        Expr *e = pslookup(ps, i);
        psput(ps, REC_EXPR(e));
    }

    for (int i = 0; i < len; i++) {
        for (int j = 0; j < len; j++) {
            /* TODO: it's not safe to point to yourself because of reallocs */
            Expr *e1 = pslookup(ps, i);
            Expr *e2 = pslookup(ps, j);

            BinOp *add = aalloc(a, sizeof(BinOp), alignof(BinOp));
            *add = ADD_BINOP(e1, e2); /* TODO: this is slow */
            psput(ps, BINOP_EXPR(add));

            BinOp *mul = aalloc(a, sizeof(BinOp), alignof(BinOp));
            *mul = MUL_BINOP(e1, e2);
            psput(ps, BINOP_EXPR(mul));
        }
    }
}

void pselim(Programs *ps, Samples *ss) {
    /* TODO */
}

void ssinit(Samples *ss, Arena *a) {
    ss->xs = aalloc(a, ARRCAP * sizeof(int), alignof(int));
    ss->ys = aalloc(a, ARRCAP * sizeof(int), alignof(int));
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
        ss->xs = arealloc(a, ss->xs, sizeof(*(ss->xs)) * 2 * ss->cap, alignof(*(ss->xs)));
        ss->ys = arealloc(a, ss->xs, sizeof(*(ss->ys)) * 2 * ss->cap, alignof(*(ss->ys)));
        ss->cap = 2 * ss->cap;
    }
    ss->xs[ss->len] = x;
    ss->ys[ss->len] = y;
    ss->len += 1;
}

Programs sssynthesize(Samples *ss, unsigned int depth, Arena *a) {
    Programs ps = {0};

    psinit(&ps, a);

    /* Add terminals to program list */
    psput(&ps, INPUT_EXPR());
    /* for (int i = -10; i <= 10; i++) */

    /* for (int i = -1; i <= 1; i++) */
    /*     psput(&ps, NUM_EXPR(i)); */
    psput(&ps, NUM_EXPR(-2));
    psput(&ps, NUM_EXPR(-1));
    psput(&ps, NUM_EXPR(1));

    for (int i = 0; i < depth; i++) {
        psgrow(&ps, a);
    }

    return ps;
}

/* TODO: add Rec expr (with base case 0) */
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

    /* printf("\nSAMPLES\n"); */
    /* printf("n = %d\n", ss.len); */
    /* for (int i = 0; i < ss.len; i++) */
    /*     printf("%d %d\n", ss.xs[i], ss.ys[i]); */

    clock_t tic = clock();

    Programs ps = sssynthesize(&ss, 3, &barena);

    clock_t toc = clock();
    unsigned int dt = (((double)(toc - tic) / CLOCKS_PER_SEC) * 1000000000); /* time in ns */

    printf("\nPROGRAMS\n");
    printf("n = %d\n", ps.len);
    printf("dt = %d\n", dt);
    for (int i = 0; i < NSHOW; i++) {
        Expr *e = pslookup(&ps, i);
        char *s = eshow(e);
        printf("%s %d %d %d\n", s, eeval(e, NULL, 0, ss.xs[0]), eeval(e, NULL, 0, ss.xs[1]), eeval(e, NULL, 0, ss.xs[2]));
        free(s);
    }

    /* for (int i = 0; i < ps.len; i++) { */
    /*     Expr *e = pslookup(&ps, i); */
    /*     char *s = eshow(e); */
    /*     printf("%s\n", s); */
    /*     if (strcmp(s, "(Add (Rec (Add Input (Num -1))) (Rec (Add Input (Num -2))))") == 0) { */
    /*         printf("FOUND FOUND FOUND"); */
    /*         free(s); */
    /*         exit(1); */
    /*     } */
    /*     free(s); */
    /* } */

    if (ps.len > NSHOW)
        printf("...\n");
    for (int i = ps.len - NSHOW; i < ps.len; i++) {
        Expr *e = pslookup(&ps, i);
        char *s = eshow(e);
        printf("%s %d %d %d\n", s, eeval(e, NULL, 0, ss.xs[0]), eeval(e, NULL, 0, ss.xs[1]), eeval(e, NULL, 0, ss.xs[2]));
        free(s);
    }

    /*
     * (eeval (Add (Rec (Add Input (Num -1)))
     *             (Num 1))
     *        NULL
     *        3)
     * (Mul (Rec (Add Input (Num -1))) Input)
     */

    /* printf("\nTEST\n"); */
    Expr einput = INPUT_EXPR();
    Expr enumneg1 = NUM_EXPR(-1);
    BinOp bopdec = ADD_BINOP(&einput, &enumneg1);
    Expr edec = BINOP_EXPR(&bopdec);
    Expr erec = REC_EXPR(&edec);

    Expr num1 = NUM_EXPR(1);

    BinOp bopadd = ADD_BINOP(&erec, &num1);

    Expr etot = BINOP_EXPR(&bopadd);

    BinOp bopfact = MUL_BINOP(&erec, &einput);
    Expr efact = BINOP_EXPR(&bopfact);

    Expr enumneg2 = NUM_EXPR(-2);
    BinOp bopdec2 = ADD_BINOP(&einput, &enumneg2);
    Expr edec2 = BINOP_EXPR(&bopdec2);
    Expr erec2 = REC_EXPR(&edec2);
    BinOp bopfib = ADD_BINOP(&erec, &erec2);
    Expr efib = BINOP_EXPR(&bopfib);

    /* printf("%s %d\n", eshow(&etot), eeval(&etot, NULL, 0, 3)); */
    /* printf("%s %d\n", eshow(&erec), eeval(&erec, NULL, 0, 3)); */
    /* printf("%s %d\n", eshow(&efact), eeval(&efact, NULL, 0, 5)); */
    /* printf("%s %d\n", eshow(&efact), eeval(&efact, NULL, 0, 6)); */
    /* printf("%s %d\n", eshow(&efib), eeval(&efib, NULL, 0, 3)); */
    /* printf("%s %d\n", eshow(&efib), eeval(&efib, NULL, 0, 5)); */
    /* printf("%s %d\n", eshow(&efib), eeval(&efib, NULL, 0, 6)); */


    /* exit(1); */

    printf("\n");
    printf("CANDIDATES\n");
    unsigned int candidates = 0;
    for (int i = 0; i < ps.len; i++) {
        /* if (i % 200000 == 0) */
        /*     printf("i = %d\n", i); */

        Expr *e = pslookup(&ps, i);

        /* if (strcmp(eshow(e), "(Add (Rec (Add Input (Num -1))) (Rec (Add Input (Num -2))))") == 0) */
        /*     printf("FOUDND FOUND FOUND %s %d %d %d %d %d\n", eshow(e), eeval(e, NULL, 0, ss.xs[0]), eeval(e, NULL, 0, ss.xs[1]), eeval(e, NULL, 0, ss.xs[2]), eeval(e, NULL, 0, ss.xs[3]), eeval(e, NULL, 0, ss.xs[4])); */

        bool verify = true;
        for (int i = 0; i < ss.len && verify; i++) {
            verify = verify && (eeval(e, NULL, 0, ss.xs[i]) == ss.ys[i]);
        }

        if (verify) {
            candidates += 1;
            if (candidates <= NSHOW) {
                char *s = eshow(e);
                printf("%s\n", s);
                free(s);
            }
            break;
        }
    }
    if (candidates > NSHOW)
        printf("...\n");
    printf("\ncandidates = %d\n", candidates);

    adeinit(&barena);
}

/*
 * TODO:
 * - maybe color or make the logo cooler?
 * - add support for piping
 * - add exiting prompt (thank you)
 * - high performance, simd optimization
 * - add tensor library
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
