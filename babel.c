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
#include "babel.h"

/* TODO: make sure the alignof's are safe */
#define alignof(type) ((size_t)&((struct { char c; type d; } *)0)->d) /* TODO: magic from stackoverflow */

Arena barena = {0}; /* babel arena */

void ainit(Arena *a) {
    a->buf = calloc(MEMCAP, 1);
    /* a->buf = mmap(NULL, MEMCAP, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0); */
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

/* TODO: Test this */
void *acalloc(Arena *a, unsigned int nbytes, unsigned int align) {
    void *ptr = aalloc(a, nbytes, align);

    memset(ptr, 0, nbytes);

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

/* TODO: fix alignment here */
void *arenamalloc(unsigned int nbytes, void *ctx) {
    Arena *a = ctx;
    return aalloc(a, nbytes, 1);
}

void arenafree(void *ptr, void *ctx) {
    /* Do nothing */
}

/* TODO: Fix alignment */
void *arenarealloc(void *ptr, unsigned int nbytes, void *ctx) {
    Arena *a = ctx;
    return arealloc(a, ptr, nbytes, 1);
}

static Allocator babel_allocator = {arenamalloc, arenafree, arenarealloc, &barena};

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
void psinit(Programs *ps, Allocator *a) {
    /* ps->buf = calloc(MAXPROGRAMS, sizeof(PROGRAM_T)); */
    /* ps->buf = aalloc(a, MAXPROGRAMS * sizeof(PROGRAM_T), alignof(PROGRAM_T)); */
    ps->buf = a->malloc(MAXPROGRAMS * sizeof(PROGRAM_T), a->ctx);
    /* ps->buf = a->malloc(MAXPROGRAMS * sizeof(PROGRAM_T), a->context); */
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

void psrandgrow(Programs *ps, unsigned int n, Allocator *a) {
    unsigned int count = 0;
    while (count++ < n) {
        unsigned int v = rand() % 3; /* TODO: use my own prng */

        if (v == 0) {
            unsigned int i = rand() % ps->len;
            Expr *e = pslookup(ps, i);
            psput(ps, REC_EXPR(e));
        } else if (v == 1) {
            unsigned int i = rand() % ps->len;
            unsigned int j = rand() % ps->len;

            Expr *e1 = pslookup(ps, i);
            Expr *e2 = pslookup(ps, j);

            /* BinOp *add = aalloc(a, sizeof(BinOp), alignof(BinOp)); */
            BinOp *add = a->malloc(sizeof(BinOp), a->ctx);
            *add = ADD_BINOP(e1, e2);
            psput(ps, BINOP_EXPR(add));
        } else if (v == 2) {
            unsigned int i = rand() % ps->len;
            unsigned int j = rand() % ps->len;

            Expr *e1 = pslookup(ps, i);
            Expr *e2 = pslookup(ps, j);

            /* BinOp *mul = aalloc(a, sizeof(BinOp), alignof(BinOp)); */
            BinOp *mul = a->malloc(sizeof(BinOp), a->ctx);
            *mul = MUL_BINOP(e1, e2);
            psput(ps, BINOP_EXPR(mul));
        }
        else
            assert(false);
    }

}

/* TODO: test changes using Allocator */
void psgrow(Programs *ps, Allocator *a) {
    unsigned int len = ps->len;

    /* TODO: add this back */
    for (int i = 0; i < len; i++) {
        Expr *e = pslookup(ps, i);
        psput(ps, REC_EXPR(e));
    }

    /* TODO: do these all in a data-directed, declarative way */
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < len; j++) {
            /* TODO: it's not safe to point to yourself because of reallocs */
            Expr *e1 = pslookup(ps, i);
            Expr *e2 = pslookup(ps, j);

            /* BinOp *add = aalloc(a, sizeof(BinOp), alignof(BinOp)); */
            BinOp *add = a->malloc(sizeof(BinOp), a->ctx);
            *add = ADD_BINOP(e1, e2); /* TODO: this is slow */
            psput(ps, BINOP_EXPR(add));

            /* BinOp *mul = aalloc(a, sizeof(BinOp), alignof(BinOp)); */
            BinOp *mul = a->malloc(sizeof(BinOp), a->ctx);
            *mul = MUL_BINOP(e1, e2);
            psput(ps, BINOP_EXPR(mul));
        }
    }
}

void pselim(Programs *ps, Samples *ss) {
    /* TODO */
}

void ssinit(Samples *ss, Allocator *a) {
    ss->xs = a->malloc(ARRCAP * sizeof(int), a->ctx);
    ss->ys = a->malloc(ARRCAP * sizeof(int), a->ctx);
    ss->cap = ARRCAP;
    ss->len = 0;
}

void ssdeinit(Samples *ss) {
    free(ss->xs);
    free(ss->ys);
    ss->cap = 0;
    ss->len = 0;
}

void ssput(Samples *ss, int x, int y, Allocator *a) {
    if (ss->len == ss->cap) {
        ss->xs = a->realloc(ss->xs, sizeof(*(ss->xs)) * 2 * ss->cap, a->ctx);
        ss->ys = a->realloc(ss->ys, sizeof(*(ss->ys)) * 2 * ss->cap, a->ctx);
        ss->cap = 2 * ss->cap;
    }
    ss->xs[ss->len] = x;
    ss->ys[ss->len] = y;
    ss->len += 1;
}

Programs sssynthesize(Samples *ss, unsigned int depth, Allocator *a) {
    Programs ps = {0};

    psinit(&ps, &babel_allocator);

    /* Add terminals to program list */
    psput(&ps, INPUT_EXPR());

    for (int i = -10; i <= 10; i++)
        psput(&ps, NUM_EXPR(i));

    for (int i = 0; i < depth; i++)
        /* psgrow(&ps, a); */
        psrandgrow(&ps, NGROW, a);

    return ps;
}

void matinit(Matrix *mat, unsigned int *shape, unsigned int ndims, Allocator *a) {
    assert(ndims == 2);

    /* mat->buf = acalloc(a, sizeof(DTYPE) * (shp->dims[0] * shp->dims[1]), alignof(DTYPE)); */
    mat->buf = a->malloc(sizeof(DTYPE) * (shape[0] * shape[1]), a->ctx);
    mat->shape = shape;
    mat->ndims = ndims;
}

float matlookup(Matrix *mat, unsigned int i, unsigned int j) {
    /* TODO */
    return mat->buf[(mat->shape[1] * i) + j];
}

void matset(Matrix *mat, unsigned int i, unsigned int j, float x) {
    mat->buf[(mat->shape[1] * i) + j] = x;
}

/* TODO */
void matmul(Matrix *A, Matrix *B, Matrix *C) {
    unsigned int N = C->shape[0];
    unsigned int M = C->shape[1];
    unsigned int K = A->shape[1];

    assert(A->shape[1] == B->shape[0]);

    for (unsigned int i = 0; i < N; i++) {
        for (unsigned int j = 0; j < M; j++) {
            float total = 0;
            for (unsigned int k = 0; k < K; k++)
                total += matlookup(A, i, k) * matlookup(B, k, j);

            matset(C, i, j, total);
        }
    }
}

int main(int argc, char *argv[]) {

    Config cfg = {0};
    unsigned int seed = time(NULL);
    for (int i = 0; i < argc; i++) {
        cfg.quiet = cfg.quiet || (strcmp(argv[i], "-q") == 0);
        cfg.filter = cfg.filter || (strcmp(argv[i], "-f") == 0);
        if (strcmp(argv[i], "-s") == 0)
            seed = atoi(argv[i + 1]);
    }
    cfg.seed = seed;

    srand(cfg.seed);

    if (!cfg.quiet)
        prologue();

    ainit(&barena);

    unsigned int N = 2;

    /* TODO: add calloc to Allocator */
    unsigned int *shape = babel_allocator.malloc(sizeof(unsigned int) * 2, babel_allocator.ctx);
    shape[0] = N;
    shape[1] = N;
    unsigned int ndims = 2;

    Matrix A = {0};
    matinit(&A, shape, ndims, &babel_allocator);

    float total = 0;
    for (int i = 0; i < A.shape[0]; i++) {
        for (int j = 0; j < A.shape[1]; j++)
            matset(&A, i, j, total++);
    }

    Matrix B = {0};
    matinit(&B, shape, ndims, &babel_allocator);
    for (int i = 0; i < B.shape[0]; i++) {
        for (int j = 0; j < B.shape[1]; j++)
            matset(&B, i, j, total--);
    }

    /* for (int i = 0; i < A.shape[0]; i++) { */
    /*     for (int j = 0; j < A.shape[1]; j++) */
    /*         printf("%f ", matlookup(&A, i, j)); */
    /*     printf("\n"); */
    /* } */

    /* printf("\n"); */
    /* for (int i = 0; i < B.shape[0]; i++) { */
    /*     for (int j = 0; j < B.shape[1]; j++) */
    /*         printf("%f ", matlookup(&B, i, j)); */
    /*     printf("\n"); */
    /* } */

    Matrix C = {0};
    matinit(&C, shape, ndims, &babel_allocator);


    clock_t t0 = clock();

    /* matmul(&A, &B, &C); */

    clock_t t1 = clock();
    unsigned long dtmatmul = (((double)(t1 - t0) / CLOCKS_PER_SEC) * 1000000000); /* time in ns */

    /* printf("dt(matmul) = %ld\n", dtmatmul); */

    /* printf("\n"); */
    /* for (int i = 0; i < C.shape[0]; i++) { */
    /*     for (int j = 0; j < C.shape[1]; j++) */
    /*         printf("%f ", matlookup(&C, i, j)); */
    /*     printf("\n"); */
    /* } */

    /* exit(0); */

    Samples ss = {0};
    ssinit(&ss, &babel_allocator);
    if (cfg.filter) {
        char line[BUFSIZE] = {0};

        while (mygetline(line, BUFSIZE) > 0) {
            char *tok;
            int x, y;

            /* TODO: extend this to > 2 tokens */
            char *s = line;
            x = atoi((tok = strsep(&s, " ")));
            y = atoi((tok = strsep(&s, " ")));
            ssput(&ss, x, y, &babel_allocator);
        }
    } else
        interact();

    /* printf("\nSAMPLES\n"); */
    /* printf("n = %d\n", ss.len); */
    /* for (int i = 0; i < ss.len; i++) */
    /*     printf("%d %d\n", ss.xs[i], ss.ys[i]); */

    clock_t tic = clock();

    Programs ps = sssynthesize(&ss, 1, &babel_allocator);

    clock_t toc = clock();
    unsigned int dt = (((double)(toc - tic) / CLOCKS_PER_SEC) * 1000000000); /* time in ns */

    printf("\nPROGRAMS\n");
    printf("n = %d\n", ps.len);
    printf("dt = %d\n", dt);

    unsigned int nshow = NSHOW <= ps.len ? NSHOW : ps.len;
    assert(nshow <= ps.len);


    for (unsigned int i = 0; i < nshow; i++) {
        Expr *e = pslookup(&ps, i);
        char *s = eshow(e);
        printf("%s %d %d %d\n", s, eeval(e, NULL, 0, ss.xs[0]), eeval(e, NULL, 0, ss.xs[1]), eeval(e, NULL, 0, ss.xs[2]));
        free(s);
    }

    if (ps.len > nshow)
        printf("...\n");
    /* for (unsigned int i = ps.len - nshow; i < ps.len; i++) { */
    /*     Expr *e = pslookup(&ps, i); */
    /*     char *s = eshow(e); */
    /*     printf("%s %d %d %d\n", s, eeval(e, NULL, 0, ss.xs[0]), eeval(e, NULL, 0, ss.xs[1]), eeval(e, NULL, 0, ss.xs[2])); */
    /*     free(s); */
    /* } */

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
        Expr *e = pslookup(&ps, i);

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
 * - add C rewrite rules
 * - efficient compression of source code using dataflow/rewrite rules?
 * - python compiler
 *
 * - 3 interfaces:
 *   - c library
 *   - interactive mode
 *   - filter mode
 *
 *   - tui mode
 *   - language model mode
 */
