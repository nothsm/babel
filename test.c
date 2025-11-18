#include <math.h>
#include <stdio.h>
#include "babel.h"
#include "test.h"

/* TODO: figure out how to increase this wrt valbwd_tanh */
#define EPS 0.000001
/* #define EPS 0.00000000000001 */

void valalloc_simple(void);
void valbwd_simple(void);
void valbwd_tanh(void);
void valeq_simple(void);

int main(int argc, char *argv[]) {
    engineinit();

    valalloc_simple();
    valbwd_simple();
    valbwd_tanh();
    valeq_simple();
}

bool feq(float x, float y) {
    return fabs(x - y) < EPS;
}

/* TODO: extend this */
void valeq_simple() {
    test("valeq_simple");

    Value v1, v2;

    valinit(&v1, VAL_FLOAT, 2.0, NULL, NULL);
    valinit(&v2, VAL_FLOAT, 2.0, NULL, NULL);
    if (!valeq(&v1, &v2))
        error("valeq_simple: values should be the same");

    pass("");
}

void valalloc_simple() {
    test("valalloc_simple");

    Value vzero = {0};
    valinit(&vzero, VAL_FLOAT, 0.0, NULL, NULL);

    unsigned int n = 3;

    Value *valloced = valalloc(n);
    for (int i = 0; i < n; i++)
        if (!valeq(&vzero, valloced + i))
            error("valalloc_simple: value should be zeroed");

    pass("");
}

void valbwd_simple() {
    test("valbwd_simple");

    Value a = {0};
    Value b = {0};
    Value c = {0};
    Value f = {0};

    valinit(&a, VAL_FLOAT, 2.0, NULL, NULL);
    valinit(&b, VAL_FLOAT, -3.0, NULL, NULL);
    valinit(&c, VAL_FLOAT, 10.0, NULL, NULL);
    Value *e = valmul(&a, &b);
    Value *d = valadd(e, &c);
    valinit(&f, VAL_FLOAT, -2.0, NULL, NULL);
    Value *L = valmul(d, &f);

    valbwd(L);

    /* TODO: is there a better way to do these? */
    if (!feq(a.grad, 6.0))
        error("valbwd_simple: gradient is incorrect (is %f, should be %f)", a.grad, 6.0);
    if (!feq(b.grad, -4.0))
        error("valbwd_simple: gradient is incorrect (is %f, should be %f)", b.grad, -4.0);
    if (!feq(c.grad, -2.0))
        error("valbwd_simple: gradient is incorrect (is %f, should be %f)", c.grad, -2.0);
    if (!feq(d->grad, -2.0))
        error("valbwd_simple: gradient is incorrect (is %f, should be %f)", d->grad, -2.0);
    if (!feq(e->grad, -2.0))
        error("valbwd_simple: gradient is incorrect (is %f, should be %f)", e->grad, -2.0);
    if (!feq(f.grad, 4.0))
        error("valbwd_simple: gradient is incorrect (is %f, should be %f)", f.grad, 4.0);
    if (!feq(L->grad, 1.0))
        error("valbwd_simple: gradient is incorrect (is %f, should be %f)", L->grad, 1.0);

    pass("");
}

void valbwd_tanh() {
    test("valbwd_tanh");

    Value x1 = {0};
    Value x2 = {0};
    Value w1 = {0};
    Value w2 = {0};
    Value b = {0};

    valinit(&x1, VAL_FLOAT, 2.0, NULL, NULL);
    valinit(&x2, VAL_FLOAT, 0.0, NULL, NULL);
    valinit(&w1, VAL_FLOAT, -3.0, NULL, NULL);
    valinit(&w2, VAL_FLOAT, 1.0, NULL, NULL);
    valinit(&b, VAL_FLOAT, 6.8813735870195432, NULL, NULL);
    Value *x1w1 = valmul(&x1, &w1);
    Value *x2w2 = valmul(&x2, &w2);
    Value *x1w1x2w2 = valadd(x1w1, x2w2);
    Value *n = valadd(x1w1x2w2, &b);
    Value *o = valtanh(n);

    valbwd(o);

    if (!feq(x1.grad, -1.5))
        error("valbwd_tanh: gradient is incorrect (is %f, should be %f)", x1.grad, -1.5);
    if (!feq(x2.grad, 0.5))
        error("valbwd_tanh: gradient is incorrect (is %f, should be %f)", x2.grad, 0.5);
    if (!feq(w1.grad, 1.0))
        error("valbwd_tanh: gradient is incorrect (is %f, should be %f)", w1.grad, 1.0);
    if (!feq(w2.grad, 0.0))
        error("valbwd_tanh: gradient is incorrect (is %f, should be %f)", w2.grad, 0.0);
    if (!feq(b.grad, 0.5))
        error("valbwd_tanh: gradient is incorrect (is %f, should be %f)", b.grad, 0.5);
    if (!feq(x1w1->grad, 0.5))
        error("valbwd_tanh: gradient is incorrect (is %f, should be %f)", x1w1->grad, 0.5);
    if (!feq(x2w2->grad, 0.5))
        error("valbwd_tanh: gradient is incorrect (is %f, should be %f)", x2w2->grad, 0.5);
    if (!feq(x1w1x2w2->grad, 0.5))
        error("valbwd_tanh: gradient is incorrect (is %f, should be %f)", x1w1x2w2->grad, 0.5);
    if (!feq(n->grad, 0.5))
        error("valbwd_tanh: gradient is incorrect (is %f, should be %f)", n->grad, 0.5);
    if (!feq(o->grad, 1.0))
        error("valbwd_tanh: gradient is incorrect (is %f, should be %f)", o->grad, 1.0);

    pass("");
}

/* TODO:
 * [ ] copy peter norvig testing style
 * [ ] add numerical gradient tests
 */
