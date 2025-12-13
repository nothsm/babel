/*
 * Author: Hilal Mufti <hilalmufti1@gmail.com>
 * Date: 12/12/2025
 *
 * this module defines the core Value data structure as well as its basic operations 
 * 
 * run tests with make test && ./test
 */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "babel.h"

char STRTAB[STRCAP];
unsigned int allocated;

Value VALTAB[VALCAP];
unsigned int vid;

extern Neuron NEURTAB[NEURCAP];
extern unsigned int nneur;

extern Layer LAYTAB[LAYCAP];
extern unsigned int nlay;

void stdbg(unsigned int beg, unsigned int end) {
    for (unsigned int i = beg; i < end; i++)
        printf("%d [%p]: %c\n", i, STRTAB + i, STRTAB[i]);
}

/* TODO: How do I make sure this is called before any other operation? */
void engineinit() {
    srand((unsigned int)time(NULL)); /* TODO: use a proper seed */

    memset(STRTAB, 0, STRCAP);
    allocated = 0;

    memset(VALTAB, 0, VALCAP);
    vid = 0;

    memset(NEURTAB, 0, NEURCAP);
    nneur = 0;

    memset(LAYTAB, 0, LAYCAP);
    nlay = 0;
}

void valcheck(Value *v) {
    assert(v != NULL);
    if (v->op == VAL_ADD)
        // assert(v->prev1 != NULL && v->prev2 != NULL); /* This is buggy because VAL_ADD is equivalent to 0 */
        assert(true);
    else if (v->op == VAL_MUL)
        assert(v->prev1 != NULL && v->prev2 != NULL);
    else if (v->op == VAL_FLOAT)
        assert(v->prev1 == NULL && v->prev2 == NULL);
    else if (v->op == VAL_TANH)
        assert(v->prev1 != NULL && v->prev2 == NULL);
    else
        assert(false);
    assert(v->prev1 != v); /* TODO: Think about this invariant */
    assert(v->prev2 != v);
    assert(v->id < vid);
    assert(vid <= VALCAP);
    assert(v->id <= VALCAP);
}

/* What if I just predefine all the id's? */
Value *valalloc(unsigned int n) {
    assert(vid + n <= VALCAP);

    unsigned int oldvid = vid;

    for (int i = 0; i < n; i++) {
        memset(VALTAB + vid, 0, sizeof(VALTAB[vid]));
        VALTAB[vid].id = vid;
        vid += 1;
    }

    assert(vid + n <= VALCAP);
    assert(VALTAB[vid - n].id == oldvid);
    for (unsigned int i = 0; i < n; i++)
        assert(VALTAB[oldvid + i].id == oldvid + i);

    return VALTAB + (vid - n);
}

/* TODO: add valfree */

void valinit(Value *v, ValueType op, float val, Value *prev1, Value *prev2) {
    assert(prev1 != v);
    assert(prev2 != v);

    v->op = op;
    v->val = val;
    v->grad = 0.0;
    v->prev1 = prev1;
    v->prev2 = prev2;

    valcheck(v);
}

bool valeq(Value *v1, Value *v2) {
    valcheck(v1);
    valcheck(v2);

    bool is_op_eq = v1->op == v2->op;
    bool is_val_eq = v1->val == v2->val;
    bool is_grad_eq = v1->grad == v2->grad;

    /* TODO: Cleanup this kludge */
    bool is_prev1_eq = (v1->prev1 == NULL && v2->prev1 == NULL)
                       || (v1->prev1 != NULL && v2->prev1 != NULL
                           && valeq(v1->prev1, v2->prev1));
    bool is_prev2_eq = (v1->prev2 == NULL && v2->prev2 == NULL)
                       || (v1->prev2 != NULL && v2->prev2 != NULL
                           && valeq(v1->prev2, v2->prev2));

    return is_op_eq
           && is_val_eq
           && is_grad_eq
           && is_prev1_eq
           && is_prev2_eq;
}

char *vtshow(ValueType vt) {
    int n;
    unsigned int unsigned_n;
    unsigned int old_allocated = allocated;

    switch (vt) {
        case VAL_ADD:
            n = snprintf(STRTAB + allocated, STRCAP, "%s", "+"); /* TODO: refactor global strtable printing */

            assert(n >= 0);

            unsigned_n = (unsigned int)n;

            assert(allocated + unsigned_n + 1 < STRCAP);

            allocated += unsigned_n + 1;
            return STRTAB + old_allocated;
        case VAL_MUL:
            n = snprintf(STRTAB + allocated, STRCAP, "%s", "*");

            assert(n >= 0);

            unsigned_n = (unsigned int)n;

            assert(allocated + unsigned_n + 1 < STRCAP);

            allocated += unsigned_n + 1;
            return STRTAB + old_allocated;
        case VAL_TANH:
            n = snprintf(STRTAB + allocated, STRCAP, "%s", "tanh");

            assert(n >= 0);

            unsigned_n = (unsigned int)n;

            assert(allocated + unsigned_n + 1 < STRCAP);

            allocated += unsigned_n + 1;
            return STRTAB + old_allocated;
        case VAL_FLOAT:
            n = snprintf(STRTAB + allocated, STRCAP, "%s", "float");

            assert(n >= 0);

            unsigned_n = (unsigned int)n;

            assert(allocated + unsigned_n + 1 < STRCAP);

            allocated += unsigned_n + 1;
            return STRTAB + old_allocated;
        default:
            assert(false);
    }
}

/* TODO: I should cache the results of this */
/* TODO: Handle self-references properly? */
/* TODO: Fix this kludge */
char *valshow(Value *v) {
    int n;
    unsigned int unsigned_n;
    unsigned int old_allocated;

    valcheck(v);

    if (v->prev1 != NULL && v->prev2 != NULL) {
        char *op = vtshow(v->op);
        char *s1 = valshow(v->prev1);
        char *s2 = valshow(v->prev2);
        old_allocated = allocated;

        n = snprintf(STRTAB + allocated, STRCAP,
                 "Value(op=%s, val=%.2f, grad=%.2f, prev1=%s, prev2=%s, id=%d)",
                 op,
                 v->val,
                 v->grad,
                 s1,
                 s2,
                 v->id);

        assert(n >= 0);

        unsigned_n = (unsigned int)n;

        assert(allocated + unsigned_n + 1 < STRCAP);

        allocated += unsigned_n + 1;
        return STRTAB + old_allocated;
    } else if (v->prev1 != NULL && v->prev2 == NULL) {
        char *op = vtshow(v->op);
        char *s = valshow(v->prev1);
        old_allocated = allocated;

        n = snprintf(STRTAB + allocated, STRCAP,
                 "Value(op=%s, val=%.2f, grad=%.2f, prev1=%s, prev2=NULL, id=%d)",
                 op,
                 v->val,
                 v->grad,
                 s,
                 v->id);

        assert(n >= 0);

        unsigned_n = (unsigned int)n;

        assert(allocated + unsigned_n + 1 < STRCAP);

        allocated += unsigned_n + 1;
        return STRTAB + old_allocated;
    } else if (v->prev1 == NULL && v->prev2 == NULL) {
        char *op = vtshow(v->op);
        old_allocated = allocated;

        n = snprintf(STRTAB + allocated, STRCAP,
                 "Value(op=%s, val=%.2f, grad=%.2f, prev1=NULL, prev2=NULL, id=%d)",
                 op,
                 v->val,
                 v->grad,
                 v->id);

        assert(n >= 0);

        unsigned_n = (unsigned int)n;

        assert(allocated + unsigned_n + 1 < STRCAP);

        allocated += unsigned_n + 1;
        return STRTAB + old_allocated;
    } else
        assert(false);
}

/* TODO: Fix this kludge */
char *valsexpr(Value *v) {
    int n;
    unsigned int unsigned_n;
    unsigned int old_allocated;

    valcheck(v);

    if (v->prev1 != NULL && v->prev2 != NULL) {
        char *op = vtshow(v->op);
        char *s1 = valsexpr(v->prev1);
        char *s2 = valsexpr(v->prev2);
        old_allocated = allocated;

        n = snprintf(STRTAB + allocated, STRCAP,
                 "(%s %s %s :val %.5f :grad %.5f)",
                 op,
                 s1,
                 s2,
                 v->val,
                 v->grad);

        assert(n >= 0);

        unsigned_n = (unsigned int)n;

        assert(allocated + unsigned_n + 1 < STRCAP);

        allocated += unsigned_n + 1;
        return STRTAB + old_allocated;
    } else if (v->prev1 != NULL && v->prev2 == NULL) {
        char *op = vtshow(v->op);
        char *s = valsexpr(v->prev1);
        old_allocated = allocated;

        n = snprintf(STRTAB + allocated, STRCAP,
                 "(%s %s :val %.5f :grad %.5f)",
                 op,
                 s,
                 v->val,
                 v->grad);

        assert(n >= 0);

        unsigned_n = (unsigned int)n;

        assert(allocated + unsigned_n + 1 < STRCAP);

        allocated += unsigned_n + 1;
        return STRTAB + old_allocated;
    } else if (v->prev1 == NULL && v->prev2 == NULL) {
        char *op = vtshow(v->op);
        old_allocated = allocated;

        n = snprintf(STRTAB + allocated, STRCAP,
                 "(%s %.5f :grad %.5f)",
                 op,
                 v->val,
                 v->grad);

        assert(n >= 0);

        unsigned_n = (unsigned int)n;

        assert(allocated + unsigned_n + 1 < STRCAP);

        allocated += unsigned_n + 1;
        return STRTAB + old_allocated;
    } else
        assert(false);
}

/* TODO */
Value *valfloats(unsigned int n, float *xs) {
    assert(n > 0);
    assert(xs != NULL);

    Value *vs = valalloc(n);
    for (int i = 0; i < n; i++)
        valinit(vs + i, VAL_FLOAT, xs[i], NULL, NULL);

    for (int i = 0; i < n; i++)
        valcheck(vs + i);

    return vs;
}

/* TODO */
Value *valfloat(float x) {
    return valfloats(1, &x);
}

Value *valadd(Value *x, Value *y) {
    valcheck(x);
    valcheck(y);

    Value *ret = valalloc(1);

    ValueType op = VAL_ADD;
    float val = x->val + y->val;
    Value *prev1 = x;
    Value *prev2 = y;

    valinit(ret, op, val, prev1, prev2);

    return ret;
}

Value *valmul(Value *x, Value *y) {
    valcheck(x);
    valcheck(y);

    Value *ret = valalloc(1);

    ValueType op = VAL_MUL;
    float val = x->val * y->val;
    Value *prev1 = x;
    Value *prev2 = y;

    valinit(ret, op, val, prev1, prev2);

    return ret;
}

Value *valsub(Value *x, Value *y) {
    valcheck(x);
    valcheck(y);

    Value *ret = valadd(x, valmul(valfloat(-1.0), y));

    valcheck(ret);

    return ret;
}

Value *valpow(Value *v, unsigned int n) {
    assert(n > 0);
    valcheck(v);

    /* TODO: Do I even need to allocate this guy? */
    Value *out = valfloat(1.0);
    for (int i = 0; i < n; i++)
        out = valmul(out, v);

    valcheck(out);

    return out;
}

Value *valtanh(Value *x) {
    valcheck(x);

    Value *ret = valalloc(1);

    ValueType op = VAL_TANH;
    float val = ((float)exp(2 * x->val) - 1) / ((float)exp(2 * x->val) + 1);
    Value *prev1 = x;
    Value *prev2 = NULL;

    valinit(ret, op, val, prev1, prev2);

    return ret;
}

void valaddbwd(Value *v) {
    valcheck(v);
    assert(v->op == VAL_ADD);

    v->prev1->grad += v->grad;
    v->prev2->grad += v->grad;

    valcheck(v);
}

void valmulbwd(Value *v) {
    valcheck(v);
    assert(v->op == VAL_MUL);

    v->prev1->grad += v->grad * v->prev2->val;
    v->prev2->grad += v->grad * v->prev1->val;

    valcheck(v);
}

void valtanhbwd(Value *v) {
    valcheck(v);
    assert(v->op == VAL_TANH);
    assert(v->prev2 == NULL);

    v->prev1->grad += v->grad * (1 - (v->val * v->val));

    valcheck(v);
}

void valbwd1(Value *v) {
    valcheck(v);

    switch (v->op) {
        case VAL_ADD:
            valaddbwd(v);
            break;
        case VAL_MUL:
            valmulbwd(v);
            break;
        case VAL_TANH:
            valtanhbwd(v);
        case VAL_FLOAT:
            break;
        default:
            assert(false);
    }

    valcheck(v);
}

void _valtsort(Value *v, Value **ret, unsigned int *nret, bool *seen) {
    if (!seen[v->id]) {
        seen[v->id] = true;

        if (v->prev1 != NULL)
            _valtsort(v->prev1, ret, nret, seen);

        if (v->prev2 != NULL)
            _valtsort(v->prev2, ret, nret, seen);

        ret[(*nret)++] = v;
    }
}

unsigned int valtsort(Value *v, Value **ret) {
    unsigned int n = 0;
    bool seen[VALCAP] = {0};

    valcheck(v);

    _valtsort(v, ret, &n, seen);

    valcheck(v);

    return n;
}

void valbwd(Value *v) {
    valcheck(v);
    assert(v->grad == 0.0); /* shouldnt use == to compare floats */

    Value *topo[VALCAP] = {0};
    unsigned int ntopo = valtsort(v, topo);

    assert(ntopo > 0);

    v->grad = 1.0;
    for (int i = (int)ntopo - 1; i >= 0; i--)
        valbwd1(topo[i]);

    valcheck(v);
}

/*
 * TODO
 * - [ ] Replace pointers with handles
 * - [ ] Add Raylib visualization
 * - [ ] Add TUI visualization/dashboard
 * - [ ] Add compilation to C backend 
 * - [ ] Support tensors
 * - [ ] feature: Add Python frontend / bindings 
 * - [ ] feature: Add Racket/Scheme frontend / bindings
 * - [ ] Use C for building (copy Tsoding or Casey Muratori)
 * - [ ] Fix warnings
 * - [ ] Allow custom allocators as parameters
 * - [ ] Add tests to verify id allocation
 * - [ ] Add proper string allocation
 * - [ ] Use doubles instead of floats for higher precision
 * - [ ] Use explicitly sized types
 * - [ ] cool feature: compile the learned algorithm of the model into code
 * - [ ] What if valfloats just took in a shape?
 * - [ ] feature: Support polyhedral compilation
 * - [ ] feature: Support multiple backends
 * - [ ] feature: allow users to specify rewrite rules
 * - [ ] feature: hand-optimized kernels for each backend
 * - [ ] feature: use linear programming for optimization
 * - [ ] feature: high-level neural network building block library
 * - [ ] feature: eager-evaluated array library
 * - [ ] feature: support high-level programming language frontends in addition to tensors
 * - [ ] feature: support input-output frontend in addition to tensors
 * - [ ] feature: fastest compiler + fastest code + rewrite rules + runs everywhere + tiny binary
 * - [ ] feature: support distributed training
 * - [ ] feature: use program synthesis + RL for the kernel search 
 * - [ ] feature: add cool concurrency/parallelism
 * - [ ] feature: program synthesis building block library
 * - [ ] feature: compile the program directly down to hardware/verilog
 * - [ ] demo: monte carlo calculations
 * - [ ] demo: interesting physics/math computations
 * - [ ] demo: easy to implement frontier papers
 * - [ ] demo: llm training
 * - [ ] demo: llm inference
 * - [ ] demo: run on something funny like a DS
 * - [ ] Fix Value printing in MLP training in main.c
 * - [ ] Add optimization + egraphs
 * - [ ] support -nostdlib
 * - [ ] Systems-level performance optimization
 *   - [ ] DoD
 *   - [ ] Pooled arena allocation
 * - [ ] Improve exceptions
 * 
 * 
 * - If you put these operations in a training loop, you don't want to allocate new memory on each iteration...
 * - Value is by-default multiple values?
 * - I don't think I actually need to compute the value in Value? I can always get it from traversing from the root
 * - Not having a length or shape is becoming painful...
 */
