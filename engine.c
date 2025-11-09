#include "babel.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct Value Value;

typedef enum {
    VAL_ADD,
    VAL_MUL,
    VAL_CONST
} ValueType;

/* Can I just make this a normal Expr? */
struct Value {
    ValueType op;
    float data;
    Value *prev1;
    Value *prev2;
};

char STRINGS[STRCAP];
unsigned int allocated;

void strsdbg(unsigned int beg, unsigned int end) {
    for (int i = beg; i < end; i++)
        printf("%d [0x%ld]: %c\n", i, STRINGS + i, STRINGS[i]);
}

void valcheck(Value *v) {
    assert(v != NULL);
    assert(v->op == VAL_ADD ||
           v->op == VAL_MUL ||
           v->op == VAL_CONST);
    assert((v->prev1 != NULL && v->prev2 != NULL) ||
           (v->prev1 != NULL && v->prev2 == NULL) ||
           (v->prev1 == NULL && v->prev2 == NULL));
    assert(v->prev1 != v); /* TODO: Think about this invariant */
    assert(v->prev2 != v);
}

Value *valinit(Value *v, ValueType op, float data, Value *prev1, Value *prev2) {
    assert(prev1 != v);
    assert(prev2 != v);

    v->op = op;
    v->data = data;
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
            n = snprintf(STRINGS + allocated, STRCAP, "%s", "add");

            assert(n >= 0);
            assert(allocated + n + 1 < STRCAP); /* TODO: make this tight */

            allocated += n + 1;
            return STRINGS + old_allocated;
        case VAL_MUL:
            n = snprintf(STRINGS + allocated, STRCAP, "%s", "mul");

            assert(n >= 0);
            assert(allocated + n + 1 < STRCAP);

            allocated += n + 1;
            return STRINGS + old_allocated;
        case VAL_CONST:
            n = snprintf(STRINGS + allocated, STRCAP, "%s", "constant");

            assert(n >= 0);
            assert(allocated + n + 1 < STRCAP);

            allocated += n + 1;
            return STRINGS + old_allocated;
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

        n = snprintf(STRINGS + allocated, STRCAP,
                 "Value(op=%s, data=%.2f, prev1=%s, prev2=%s)",
                 op,
                 v->data,
                 s1,
                 s2);

        assert(n >= 0);
        assert(allocated + n + 1 < STRCAP);

        allocated += n + 1;
        return STRINGS + old_allocated;
    } else if (v->prev1 != NULL && v->prev2 == NULL) {
        char *op = vtshow(v->op);
        char *s = valshow(v->prev1);
        old_allocated = allocated;

        n = snprintf(STRINGS + allocated, STRCAP,
                 "Value(op=%s, data=%.2f, prev1=%s, prev2=NULL)",
                 op,
                 v->data,
                 s);

        assert(n >= 0);
        assert(allocated + n + 1 < STRCAP);

        allocated += n + 1;
        return STRINGS + old_allocated;
    } else if (v->prev1 == NULL && v->prev2 == NULL) {
        char *op = vtshow(v->op);
        old_allocated = allocated;

        n = snprintf(STRINGS + allocated, STRCAP,
                 "Value(op=%s, data=%.2f, prev1=NULL, prev2=NULL)",
                 op,
                 v->data);

        assert(n >= 0);
        assert(allocated + n + 1 < STRCAP);

        allocated += n + 1;
        return STRINGS + old_allocated;
    } else
        assert(false);
}

char *valsexpr(Value *v) {
    int n;
    unsigned int old_allocated;

    char *s = vtshow(v->op);
    old_allocated = allocated;
    n = snprintf(STRINGS + allocated, STRCAP, "(%s %.2f)", s, v->data);

    assert(n >= 0);
    assert(allocated + n + 1 < STRCAP);

    allocated += n + 1;

    return STRINGS + old_allocated;
}

Value *valadd(Value *x, Value *y, Value *ret) {
    valcheck(x);
    valcheck(y);
    assert(ret != NULL);

    valinit(ret, VAL_ADD, x->data + y->data, x, y);
    return ret;
}

Value *valmul(Value *x, Value *y, Value *ret) {
    valcheck(x);
    valcheck(y);
    assert(ret != NULL);

    valinit(ret, VAL_MUL, x->data * y->data, x, y);
    return ret;
}

float f(float x) {
    return 3 * x * x - 4 * x + 5;
}

int main(int argc, char **argv) {
    memset(STRINGS, 0, STRCAP);
    allocated = 0;

    printf("%.3f\n", f(3.0));

    Value a = {0};
    Value b = {0};
    Value c = {0};
    Value ret = {0};
    Value ret2 = {0};

    valinit(&a, VAL_CONST, 2.0, NULL, NULL);
    valinit(&b, VAL_CONST, -3.0, NULL, NULL);
    valinit(&c, VAL_CONST, 10.0, NULL, NULL);

    printf("a = %s\n", valshow(&a));

    printf("b = %s\n", valshow(&b));

    printf("c = %s\n", valshow(&c));

    printf("a + b = %s\n", valshow(valadd(&a, &b, &ret)));

    printf("a * b = %s\n", valshow(valmul(&a, &b, &ret)));

    /* Value(op=add, data=-1.00, prev1=Value(op=mul, data=-6.00, prev1=Value(op=constant, data=2.00, prev1=NULL, prev2=NULL), prev2=Value(op=constant, data=-3.00, prev1=NULL, prev2=NULL)), prev2=Value(op=constant, data=10.00, prev1=NULL, prev2=NULL)) */
    /* valadd(valmul(&a, &b, &ret), &c, &ret); */
    printf("a * b + c = %s\n", valshow(valadd(valmul(&a, &b, &ret), &c, &ret2)));

    strsdbg(0, 1024);

}

/*
 * TODO
 * - [ ] Add Raylib visualization
 * - [ ] Add TUI visualization
 * - [ ] Add C compilation
 * - [ ] Support tensors
 * - [ ] Add Python bindings
 * - [ ] Add Racket/Scheme bindings
 * - [ ] Systems-level performance optimization
 *   - [ ] DoD
 *   - [ ] Pooled arena allocation
 */
