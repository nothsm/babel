#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "babel.h"
#include "test.h"

/* TODO: figure out how to increase this wrt valbwd_tanh */
#define EPS 0.000001
/* #define EPS 0.00000000000001 */

void valalloc_basic(void);
void valinit_basic(void);
// void valfloat_basic(void);
void valbwd_basic(void);
void valbwd_tanh(void);
void valbwd_selfref(void);
void valeq_basic(void);

void nalloc_basic(void);
void nfwd_basic(void);

void lfwd_basic(void);
void lparams_basic(void);

/* I think I have to refactor all my backward tests to use valalloc */
int main(int argc, char *argv[]) {
    engineinit();

    valalloc_basic();
    valinit_basic();
    // valfloat_basic();
    valbwd_basic();
    valbwd_tanh();
    valbwd_selfref();
    valeq_basic();

    nalloc_basic();
    nfwd_basic();

    lfwd_basic();
    lparams_basic();
}

bool feq(float x, float y) {
    return fabs(x - y) < EPS;
}

void valalloc_basic() {
    test("valalloc_basic");

    Value *vzero = valalloc(1);

    valinit(vzero, 0, 0.0, NULL, NULL);

    unsigned int n = 3;
    Value *valloced = valalloc(n);
    for (int i = 0; i < n; i++)
        if (!valeq(vzero, valloced + i))
            error("valalloc_basic: value should be zeroed");

    pass("");
}

void valinit_basic() {
    test("valinit_basic");

    Value v1 = {0};
    Value v2 = {0};
    Value v3 = {0};

    valinit(&v1, VAL_FLOAT, 1.0, NULL, NULL);
    valinit(&v2, VAL_FLOAT, 2.0, NULL, NULL);
    valinit(&v3, VAL_ADD, 3.0, &v1, &v2);

    if (v1.op != VAL_FLOAT)
        error("valinit_basic: bad initialization");
    if (v1.val != 1.0)
        error("valinit_basic: bad initialization");
    if (v1.grad != 0.0)
        error("valinit_basic: bad initialization");
    if (v1.prev1 != NULL)
        error("valinit_basic: bad initialization");
    if (v1.prev2 != NULL)
        error("valinit_basic: bad initialization");

    if (v2.op != VAL_FLOAT)
        error("valinit_basic: bad initialization");
    if (v2.val != 2.0)
        error("valinit_basic: bad initialization");
    if (v2.grad != 0.0)
        error("valinit_basic: bad initialization");
    if (v2.prev1 != NULL)
        error("valinit_basic: bad initialization");
    if (v2.prev2 != NULL)
        error("valinit_basic: bad initialization");

    if (v3.op != VAL_ADD)
        error("valinit_basic: bad initialization");
    if (v3.val != 3.0)
        error("valinit_basic: bad initialization");
    if (v3.grad != 0.0)
        error("valinit_basic: bad initialization");
    if (v3.prev1 != &v1)
        error("valinit_basic: bad initialization");
    if (v3.prev2 != &v2)
        error("valinit_basic: bad initialization");

    pass("");
}

/* TODO: extend this */
void valeq_basic() {
    test("valeq_basic");

    Value *v1 = valalloc(1);
    Value *v2 = valalloc(1);

    valinit(v1, VAL_FLOAT, 2.0, NULL, NULL);
    valinit(v2, VAL_FLOAT, 2.0, NULL, NULL);

    if (!valeq(v1, v2))
        error("valeq_basic: values should be the same");

    pass("");
}

void valbwd_basic() {
    test("valbwd_basic");

    Value *a = valalloc(1);
    Value *b = valalloc(1);
    Value *c = valalloc(1);
    Value *f = valalloc(1);

    valinit(a, VAL_FLOAT, 2.0, NULL, NULL);
    valinit(b, VAL_FLOAT, -3.0, NULL, NULL);
    valinit(c, VAL_FLOAT, 10.0, NULL, NULL);
    valinit(f, VAL_FLOAT, -2.0, NULL, NULL);

    Value *e = valmul(a, b);
    Value *d = valadd(e, c);
    Value *L = valmul(d, f);

    valbwd(L);

    /* TODO: is there a better way to do these? */
    if (!feq(a->grad, 6.0))
        error("valbwd_basic: gradient is incorrect (is %f, should be %f)", a->grad, 6.0);
    if (!feq(b->grad, -4.0))
        error("valbwd_basic: gradient is incorrect (is %f, should be %f)", b->grad, -4.0);
    if (!feq(c->grad, -2.0))
        error("valbwd_basic: gradient is incorrect (is %f, should be %f)", c->grad, -2.0);
    if (!feq(d->grad, -2.0))
        error("valbwd_basic: gradient is incorrect (is %f, should be %f)", d->grad, -2.0);
    if (!feq(e->grad, -2.0))
        error("valbwd_basic: gradient is incorrect (is %f, should be %f)", e->grad, -2.0);
    if (!feq(f->grad, 4.0))
        error("valbwd_basic: gradient is incorrect (is %f, should be %f)", f->grad, 4.0);
    if (!feq(L->grad, 1.0))
        error("valbwd_basic: gradient is incorrect (is %f, should be %f)", L->grad, 1.0);

    pass("");
}

void valbwd_tanh() {
    test("valbwd_tanh");

    Value *x1 = valalloc(1);
    Value *x2 = valalloc(1);
    Value *w1 = valalloc(1);
    Value *w2 = valalloc(1);
    Value *b  = valalloc(1);

    valinit(x1, VAL_FLOAT, 2.0, NULL, NULL);
    valinit(x2, VAL_FLOAT, 0.0, NULL, NULL);
    valinit(w1, VAL_FLOAT, -3.0, NULL, NULL);
    valinit(w2, VAL_FLOAT, 1.0, NULL, NULL);
    valinit(b, VAL_FLOAT, 6.8813735870195432, NULL, NULL);

    Value *x1w1 = valmul(x1, w1);
    Value *x2w2 = valmul(x2, w2);
    Value *x1w1x2w2 = valadd(x1w1, x2w2);
    Value *n = valadd(x1w1x2w2, b);
    Value *o = valtanh(n);

    valbwd(o);

    if (!feq(x1->grad, -1.5))
        error("valbwd_tanh: gradient is incorrect (is %f, should be %f)", x1->grad, -1.5);
    if (!feq(x2->grad, 0.5))
        error("valbwd_tanh: gradient is incorrect (is %f, should be %f)", x2->grad, 0.5);
    if (!feq(w1->grad, 1.0))
        error("valbwd_tanh: gradient is incorrect (is %f, should be %f)", w1->grad, 1.0);
    if (!feq(w2->grad, 0.0))
        error("valbwd_tanh: gradient is incorrect (is %f, should be %f)", w2->grad, 0.0);
    if (!feq(b->grad, 0.5))
        error("valbwd_tanh: gradient is incorrect (is %f, should be %f)", b->grad, 0.5);
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

void valbwd_selfref() {
    test("valbwd_selfref");

    /* repeated tanh */

    Value *x = valalloc(1);

    valinit(x, VAL_FLOAT, 1.0, NULL, NULL); 

    for (int i = 0; i < 3; i++)
        x = valtanh(x);

    if (!feq(x->val, 0.5662699762))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", x->val, 0.566269976);
    if (!feq(x->prev1->val, 0.642014992))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", x->prev1->val, 0.642014992);
    if (!feq(x->prev1->prev1->val, 0.761594156))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", x->prev1->prev1->val, 0.761594156);
    if (!feq(x->prev1->prev1->prev1->val, 1.0))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", x->prev1->prev1->prev1->val, 1.0);

    valbwd(x);

    if (!feq(x->grad, 1.0))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", x->grad, 1.0);
    if (!feq(x->prev1->grad, 0.6793383143))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", x->prev1->grad, 0.6793383143);
    if (!feq(x->prev1->prev1->grad, 0.3993264401))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", x->prev1->prev1->grad, 0.3993264401);
    if (!feq(x->prev1->prev1->prev1->grad, 0.1677068588))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", x->prev1->prev1->prev1->grad, 0.1677068588);

    /* repeated add */

    Value *xs = valalloc(3);
    Value *out = valalloc(1);

    valinit(xs + 0, VAL_FLOAT, 1.0, NULL, NULL);
    valinit(xs + 1, VAL_FLOAT, 2.0, NULL, NULL);
    valinit(xs + 2, VAL_FLOAT, 3.0, NULL, NULL);

    valinit(out, VAL_FLOAT, 0.0, NULL, NULL);

    for (int i = 0; i < 3; i++)
        out = valadd(out, xs + i);

    /* check intermediate out values */
    if (!feq(out->val, 6.0))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", out->val, 6.0);
    if (!feq(out->prev1->val, 3.0))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", out->prev1->val, 3.0);
    if (!feq(out->prev1->prev1->val, 1.0))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", out->prev1->prev1->val, 1.0);
    if (!feq(out->prev1->prev1->prev1->val, 0.0))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", out->prev1->prev1->prev1->val, 0.0);

    /* check x[2], x[1], x[0] */
    if (!feq(out->prev2->val, 3.0))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", out->prev2->val, 3.0);
    if (!feq(out->prev1->prev2->val, 2.0))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", out->prev1->prev2->val, 2.0);
    if (!feq(out->prev1->prev1->prev2->val, 1.0))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", out->prev1->prev1->prev2->val, 0.0);

    valbwd(out);

    /* check intermediate out values */
    if (!feq(out->grad, 1.0))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", out->grad, 1.0);
    if (!feq(out->prev1->grad, 1.0))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", out->prev1->grad, 1.0);
    if (!feq(out->prev1->prev1->grad, 1.0))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", out->prev1->prev1->grad, 1.0);
    if (!feq(out->prev1->prev1->prev1->grad, 1.0))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", out->prev1->prev1->prev1->grad, 1.0);

    /* check x[2], x[1], x[0] */
    if (!feq(out->prev2->grad, 1.0))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", out->prev2->grad, 1.0);
    if (!feq(out->prev1->prev2->grad, 1.0))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", out->prev1->prev2->grad, 1.0);
    if (!feq(out->prev1->prev1->prev2->grad, 1.0))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", out->prev1->prev1->prev2->grad, 1.0);

    /* repeated mul */

    xs = valalloc(3);
    out = valalloc(1);

    valinit(xs + 0, VAL_FLOAT, 1.0, NULL, NULL);
    valinit(xs + 1, VAL_FLOAT, 2.0, NULL, NULL);
    valinit(xs + 2, VAL_FLOAT, 3.0, NULL, NULL);

    valinit(out, VAL_FLOAT, 1.0, NULL, NULL);

    for (int i = 0; i < 3; i++)
        out = valmul(out, xs + i);

    /* check intermediate out values */
    if (!feq(out->val, 6.0))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", out->val, 6.0);
    if (!feq(out->prev1->val, 2.0))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", out->prev1->val, 2.0);
    if (!feq(out->prev1->prev1->val, 1.0))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", out->prev1->prev1->val, 1.0);
    if (!feq(out->prev1->prev1->prev1->val, 1.0))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", out->prev1->prev1->prev1->val, 1.0);

    /* check x[2], x[1], x[0] */
    if (!feq(out->prev2->val, 3.0))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", out->prev2->val, 3.0);
    if (!feq(out->prev1->prev2->val, 2.0))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", out->prev1->prev2->val, 2.0);
    if (!feq(out->prev1->prev1->prev2->val, 1.0))
        error("valbwd_selfref: value is incorrect (is %f, should be %f)", out->prev1->prev1->prev2->val, 0.0);

    valbwd(out);

    /* check intermediate out values */
    if (!feq(out->grad, 1.0))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", out->grad, 1.0);
    if (!feq(out->prev1->grad, 3.0))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", out->prev1->grad, 3.0);
    if (!feq(out->prev1->prev1->grad, 6.0))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", out->prev1->prev1->grad, 6.0);
    if (!feq(out->prev1->prev1->prev1->grad, 6.0))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", out->prev1->prev1->prev1->grad, 6.0);

    /* check x[2], x[1], x[0] */
    if (!feq(out->prev2->grad, 2.0))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", out->prev2->grad, 2.0);
    if (!feq(out->prev1->prev2->grad, 3.0))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", out->prev1->prev2->grad, 3.0);
    if (!feq(out->prev1->prev1->prev2->grad, 6.0))
        error("valbwd_selfref: grad is incorrect (is %f, should be %f)", out->prev1->prev1->prev2->grad, 6.0);

    /* TODO: mixed op tests */

    pass("");
}

void nalloc_basic() {
    test("nalloc_basic");

    printf(" ** warning: not implemented ** ");

    pass("");
}

void nfwd_basic() {
    test("nfwd_basic");

    unsigned int nin = 3;
    Neuron *n = nalloc(1, 3);

    ninit(n);

    for (int i = 0; i < nin; i++)
        valinit(n->w + i, VAL_FLOAT, pow(-1, i) * (i + 1.0) / 10.0, NULL, NULL);

    Value *x = valalloc(3);
    valinit(x + 0, VAL_FLOAT, 2.0, NULL, NULL);
    valinit(x + 1, VAL_FLOAT, 3.0, NULL, NULL);
    valinit(x + 2, VAL_FLOAT, -1.0, NULL, NULL);

    Value *out = nfwd(n, x);

    if (!feq(out->val, -0.6043677771))
        error("nfwd_basic: forward pass incorrect (is %f, should be %f)", out->val, -0.6043677771);
    if (!feq(out->grad, 0.0))
        error("nfwd_basic: gradient should be 0 after forward pass");

    pass("");
}

void lfwd_basic() {
    test("lfwd_basic");

    unsigned int nin = 3;
    unsigned int nout = 2;
    Layer *l = lalloc(1, nin, nout);

    linit(l);

    /* initialize first neuron to [0.1, -0.2, 0.3] */
    for (int i = 0; i < nin; i++)
        valinit(l->ns[0].w + i, VAL_FLOAT, pow(-1, i) * (i + 1.0) / 10.0, NULL, NULL);
    
    /* initialize second neuron to [-0.1, 0.2, -0.3] */
    for (int i = 0; i < nin; i++)
        valinit(l->ns[1].w + i, VAL_FLOAT, pow(-1, (i + 1)) * (i + 1.0) / 10.0, NULL, NULL);
 

    Value x[3] = {0};
    valinit(x + 0, VAL_FLOAT, 2.0, NULL, NULL);
    valinit(x + 1, VAL_FLOAT, 3.0, NULL, NULL);
    valinit(x + 2, VAL_FLOAT, -1.0, NULL, NULL);

    Value *out[2] = {0};
    lfwd(l, x, out);

    if (!feq(out[0]->val, -0.6043677771))
        error("lfwd_basic: forward pass incorrect (is %f, should be %f)", out[0]->val, -0.6043677771);
    if (!feq(out[1]->val, 0.6043677771))
        error("lfwd_basic: forward pass incorrect (is %f, should be %f)", out[0]->val, 0.6043677771);

    pass("");
}

/* TODO: This doesn't check the bias */
void lparams_basic() {
    test("lparams_basic");

    unsigned int nin = 3;
    unsigned int nout = 2;
    Layer *l = lalloc(1, nin, nout);

    linit(l);

    /* initialize first neuron to [0.1, -0.2, 0.3] */
    for (int i = 0; i < nin; i++)
        valinit(l->ns[0].w + i, VAL_FLOAT, pow(-1, i) * (i + 1.0) / 10.0, NULL, NULL);
 
    /* initialize second neuron to [-0.1, 0.2, -0.3] */
    for (int i = 0; i < nin; i++)
        valinit(l->ns[1].w + i, VAL_FLOAT, pow(-1, (i + 1)) * (i + 1.0) / 10.0, NULL, NULL);

    Value *ret[VALCAP] = {0};
    unsigned int n = lparams(l, ret);

    if (!(n == 8))
        error("lparams_basic: number of parameters is incorrect (is %d, should be %d)", n, 8);

    if (!feq(ret[0]->val, 0.1))
        error("lparams_basic: param is incorrect (is %f, should be %f)", ret[0]->val, 0.1);
    if (!feq(ret[1]->val, -0.2))
        error("lparams_basic: param is incorrect (is %f, should be %f)", ret[1]->val, -0.2);
    if (!feq(ret[2]->val, 0.3))
        error("lparams_basic: param is incorrect (is %f, should be %f)", ret[2]->val, 0.3);
    if (!feq(ret[4]->val, -0.1))
        error("lparams_basic: param is incorrect (is %f, should be %f)", ret[3]->val, -0.1);
    if (!feq(ret[5]->val, 0.2))
        error("lparams_basic: param is incorrect (is %f, should be %f)", ret[4]->val, 0.2);
    if (!feq(ret[6]->val, -0.3))
        error("lparams_basic: param is incorrect (is %f, should be %f)", ret[5]->val, -0.3);

    pass("");
}

/* TODO:
 * [ ] copy peter norvig testing style
 * [ ] add numerical gradient tests
 * [ ] Print how many test cases are run/passed/failed
 */
