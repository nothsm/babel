/*
 * try it with: make engine && ./engine
 */

#include "babel.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct Value Value;

typedef enum {
    VAL_ADD,
    VAL_MUL,
    VAL_FLOAT
} ValueType;

/* Can I just make this a normal Expr? */
struct Value {
    unsigned int id;
    ValueType op;
    float val;
    float grad;
    Value *prev1;
    Value *prev2;
};

char STRTAB[STRCAP];
unsigned int allocated;

unsigned int vid;

void stdbg(unsigned int beg, unsigned int end) {
    for (int i = beg; i < end; i++)
        printf("%d [0x%ld]: %c\n", i, STRTAB + i, STRTAB[i]);
}

void valcheck(Value *v) {
    assert(v != NULL);
    assert(v->op == VAL_ADD ||
           v->op == VAL_MUL ||
           v->op == VAL_FLOAT);
    assert((v->prev1 != NULL && v->prev2 != NULL) ||
           (v->prev1 != NULL && v->prev2 == NULL) ||
           (v->prev1 == NULL && v->prev2 == NULL));
    assert(v->prev1 != v); /* TODO: Think about this invariant */
    assert(v->prev2 != v);
    assert(v->id < vid);
}

Value *valinit(Value *v, ValueType op, float val, Value *prev1, Value *prev2) {
    assert(prev1 != v);
    assert(prev2 != v);
    assert(vid < VALCAP);

    v->id = vid++;
    v->op = op;
    v->val = val;
    v->grad = 0.0;
    v->prev1 = prev1;
    v->prev2 = prev2;

    valcheck(v);

    return v;
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
char *valshow(Value *v) {
    int n;
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

char *valsexpr(Value *v) {
    int n;
    unsigned int old_allocated;

    valcheck(v);

    if (v->prev1 != NULL && v->prev2 != NULL) {
        char *op = vtshow(v->op);
        char *s1 = valsexpr(v->prev1);
        char *s2 = valsexpr(v->prev2);
        old_allocated = allocated;

        n = snprintf(STRTAB + allocated, STRCAP,
                 "(%s %s %s :val %.2f :grad %.2f)",
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
                 "(%s %s :val %.2f :grad %.2f)",
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
                 "(%s %.2f :grad %.2f)",
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

Value *valadd(Value *x, Value *y, Value *ret) {
    valcheck(x);
    valcheck(y);
    assert(ret != NULL);

    valinit(ret, VAL_ADD, x->val + y->val, x, y);
    return ret;
}

Value *valmul(Value *x, Value *y, Value *ret) {
    valcheck(x);
    valcheck(y);
    assert(ret != NULL);

    valinit(ret, VAL_MUL, x->val * y->val, x, y);
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

void valbwd1(Value *v) {
    valcheck(v);

    switch (v->op) {
        case VAL_ADD:
            valaddbwd(v);
            break;
        case VAL_MUL:
            valmulbwd(v);
            break;
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

    Value *topo[VALCAP] = {0};
    unsigned int ntopo = valtsort(v, topo);

    v->grad = 1.0;
    for (int i = ntopo - 1; i >= 0; i--)
        valbwd1(topo[i]);

    valcheck(v);
}

float f(float x) {
    return 3 * x * x - 4 * x + 5;
}

int main(int argc, char **argv) {
    memset(STRTAB, 0, STRCAP);
    allocated = 0;

    vid = 0;

    printf("%.3f\n", f(3.0));

    Value a = {0};
    Value b = {0};
    Value c = {0};
    Value d = {0};
    Value e = {0};
    Value f = {0};
    Value L = {0};

    valinit(&a, VAL_FLOAT, 2.0, NULL, NULL);
    valinit(&b, VAL_FLOAT, -3.0, NULL, NULL);
    valinit(&c, VAL_FLOAT, 10.0, NULL, NULL);
    e = *valmul(&a, &b, &e);
    d = *valadd(&e, &c, &d);
    valinit(&f, VAL_FLOAT, -2.0, NULL, NULL);
    L = *valmul(&d, &f, &L);

    printf("a = %s\n", valsexpr(&a));
    printf("b = %s\n", valsexpr(&b));
    printf("c = %s\n", valsexpr(&c));
    printf("e = %s\n", valsexpr(&e));
    printf("d = %s\n", valsexpr(&d));
    printf("f = %s\n", valsexpr(&f));
    printf("L = %s\n", valsexpr(&L));

    printf("\n --- backpropagating... ---\n\n");
    valbwd(&L);

    printf("a = %s\n", valsexpr(&a));
    printf("b = %s\n", valsexpr(&b));
    printf("c = %s\n", valsexpr(&c));
    printf("e = %s\n", valsexpr(&e));
    printf("d = %s\n", valsexpr(&d));
    printf("f = %s\n", valsexpr(&f));
    printf("L = %s\n", valsexpr(&L));
    printf("\n");

}

/*
 * TODO
 * - [ ] Add Raylib visualization
 * - [ ] Add TUI visualization
 * - [ ] Add C compilation
 * - [ ] Support tensors
 * - [ ] Add Python bindings
 * - [ ] Add Racket/Scheme bindings
 * - [ ] Setup tests
 * - [ ] Systems-level performance optimization
 *   - [ ] DoD
 *   - [ ] Pooled arena allocation
 */
