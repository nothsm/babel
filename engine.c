/*
 * run with: make test && ./test
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
    for (int i = beg; i < end; i++)
        printf("%d [%p]: %c\n", i, STRTAB + i, STRTAB[i]);
}

/* TODO: How do I make sure this is called before any other operation? */
void engineinit() {
    srand(time(NULL)); /* TODO: use a proper seed */

    memset(STRTAB, 0, STRCAP);
    allocated = 0;

    memset(VALTAB, 0, VALCAP);
    vid = 0;

    memset(NEURTAB, 0, NEURCAP);
    nneur = 0;

    memset(LAYTAB, 0, LAYCAP);
    nlay = 0;
}

void valassert(Value *v) {
    assert(v != NULL);
    assert(v->op == VAL_ADD ||
           v->op == VAL_MUL ||
           v->op == VAL_FLOAT ||
           v->op == VAL_TANH);
    assert((v->prev1 != NULL && v->prev2 != NULL) ||
           (v->prev1 != NULL && v->prev2 == NULL) ||
           (v->prev1 == NULL && v->prev2 == NULL));
    assert(v->prev1 != v); /* TODO: Think about this invariant */
    assert(v->prev2 != v);
    assert(v->id < vid);
}

/* TODO: When should I allocate id's, if anytime? */

Value *valalloc(unsigned int n) {
    assert(vid + n <= VALCAP);

    unsigned int oldvid = vid;

    Value *ret = VALTAB + vid;
    for (int i = 0; i < n; i++) {
        Value *v = VALTAB + vid;

        memset(v, 0, sizeof(*v));
        v->id = vid;
        vid += 1;
    }

    assert(ret->id == oldvid);
    for (int i = 0; i < n; i++)
        assert(VALTAB[oldvid + i].id == oldvid + i);

    return ret;
}

/* TODO: add valfree */

Value *valinit(Value *v, ValueType op, float val, Value *prev1, Value *prev2) {
    assert(prev1 != v);
    assert(prev2 != v);

    v->op = op;
    v->val = val;
    v->grad = 0.0;
    v->prev1 = prev1;
    v->prev2 = prev2;

    valassert(v);

    return v;
}

bool valeq(Value *v1, Value *v2) {
    valassert(v1);
    valassert(v2);

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
    unsigned int old_allocated = allocated;

    switch (vt) {
        case VAL_ADD:
            n = snprintf(STRTAB + allocated, STRCAP, "%s", "+"); /* TODO: refactor global strtable printing */

            assert(n >= 0);
            assert(allocated + n + 1 < STRCAP);

            allocated += n + 1;
            return STRTAB + old_allocated;
        case VAL_MUL:
            n = snprintf(STRTAB + allocated, STRCAP, "%s", "*");

            assert(n >= 0);
            assert(allocated + n + 1 < STRCAP);

            allocated += n + 1;
            return STRTAB + old_allocated;
        case VAL_TANH:
            n = snprintf(STRTAB + allocated, STRCAP, "%s", "tanh");

            assert(n >= 0);
            assert(allocated + n + 1 < STRCAP);

            allocated += n + 1;
            return STRTAB + old_allocated;
        case VAL_FLOAT:
            n = snprintf(STRTAB + allocated, STRCAP, "%s", "float");

            assert(n >= 0);
            assert(allocated + n + 1 < STRCAP);

            allocated += n + 1;
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
    unsigned int old_allocated;

    valassert(v);

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
        assert(allocated + n + 1 < STRCAP);

        allocated += n + 1;
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
        assert(allocated + n + 1 < STRCAP);

        allocated += n + 1;
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
        assert(allocated + n + 1 < STRCAP);

        allocated += n + 1;
        return STRTAB + old_allocated;
    } else
        assert(false);
}

/* TODO: Fix this kludge */
char *valsexpr(Value *v) {
    int n;
    unsigned int old_allocated;

    valassert(v);

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
        assert(allocated + n + 1 < STRCAP);

        allocated += n + 1;
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
        assert(allocated + n + 1 < STRCAP);

        allocated += n + 1;
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
        assert(allocated + n + 1 < STRCAP);

        allocated += n + 1;
        return STRTAB + old_allocated;
    } else
        assert(false);
}

Value *valadd(Value *x, Value *y) {
    valassert(x);
    valassert(y);

    Value *ret = valalloc(1);

    ValueType op = VAL_ADD;
    float val = x->val + y->val;
    Value *prev1 = x;
    Value *prev2 = y;

    valinit(ret, op, val, prev1, prev2);

    return ret;
}

Value *valmul(Value *x, Value *y) {
    valassert(x);
    valassert(y);

    Value *ret = valalloc(1);

    ValueType op = VAL_MUL;
    float val = x->val * y->val;
    Value *prev1 = x;
    Value *prev2 = y;

    valinit(ret, op, val, prev1, prev2);

    return ret;
}

Value *valtanh(Value *x) {
    valassert(x);

    Value *ret = valalloc(1);

    ValueType op = VAL_TANH;
    float val = (exp(2 * x->val) - 1) / (exp(2 * x->val) + 1);
    Value *prev1 = x;
    Value *prev2 = NULL;

    valinit(ret, op, val, prev1, prev2);

    return ret;
}

void valaddbwd(Value *v) {
    valassert(v);
    assert(v->op == VAL_ADD);

    v->prev1->grad += v->grad;
    v->prev2->grad += v->grad;

    valassert(v);
}

void valmulbwd(Value *v) {
    valassert(v);
    assert(v->op == VAL_MUL);

    v->prev1->grad += v->grad * v->prev2->val;
    v->prev2->grad += v->grad * v->prev1->val;

    valassert(v);
}

void valtanhbwd(Value *v) {
    valassert(v);
    assert(v->op == VAL_TANH);
    assert(v->prev2 == NULL);

    float t = v->val;

    v->prev1->grad += v->grad * (1 - (t * t));

    valassert(v);
}

void valbwd1(Value *v) {
    valassert(v);

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

    valassert(v);
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

    valassert(v);

    _valtsort(v, ret, &n, seen);

    valassert(v);

    return n;
}

void valbwd(Value *v) {
    valassert(v);

    Value *topo[VALCAP] = {0};
    unsigned int ntopo = valtsort(v, topo);

    v->grad = 1.0;
    for (int i = ntopo - 1; i >= 0; i--)
        valbwd1(topo[i]);

    valassert(v);
}

/*
 * TODO
 * - [ ] Make it easier to construct and forward pass datasets
 * - [ ] Check valinit id allocations
 * - [ ] Replace pointers with handles
 * - [ ] Add Raylib visualization
 * - [ ] Add TUI visualization
 * - [ ] Add C compilation
 * - [ ] Support tensors
 * - [ ] Add Python bindings
 * - [ ] Add Racket/Scheme bindings
 * - [ ] Setup tests
 * - [ ] Use C for building (copy Tsoding or Casey Muratori)
 * - [ ] Fix warnings
 * - [ ] Fix main.c
 * - [ ] Systems-level performance optimization
 *   - [ ] DoD
 *   - [ ] Pooled arena allocation
 */
