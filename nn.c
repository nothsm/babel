#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "babel.h"

#define frand() ((double) rand() / (RAND_MAX + 1.0))

Neuron NEURTAB[NEURCAP];
unsigned int nneur;

Layer LAYTAB[LAYCAP];
unsigned int nlay;

extern char STRTAB[STRCAP];
extern unsigned int allocated;

MLP themlp = {0};

double uniform(int lo, int hi) {
    assert(lo < hi);

    return ((hi - lo) * frand()) + lo;
}

void ncheck(Neuron *n) {
    assert(n != NULL);
    assert(n->nin > 0);
}

Neuron *nalloc(unsigned int n, unsigned int nin) {
    assert(nneur + n <= NEURCAP);

    Neuron *ret = NEURTAB + nneur;
    nneur += n;

    for (int i = 0; i < n; i++) {
        /* TODO: allocate w, b as a single array? */
        ret[i].w = valalloc(nin);
        ret[i].b = valalloc(1);
        ret[i].nin = nin;
    }

    return ret;
}

void ninit(Neuron *n) {
    assert(n != NULL);

    for (int i = 0; i < n->nin; i++)
        valinit(n->w + i, VAL_FLOAT, uniform(-1, 1), NULL, NULL);
    valinit(n->b, VAL_FLOAT, uniform(-1, 1), NULL, NULL);

    ncheck(n);
}

/* TODO: generalize this to > 2 input dims */
char *nshow(Neuron *n) {
    unsigned int nalloc;
    unsigned int oldalloc;
    char *s;

    assert(n->nin >= 2);
    ncheck(n);

    oldalloc = allocated;

    nalloc = snprintf(STRTAB + allocated, STRCAP, "Neuron(w=[%.3f, %.3f], b=%.3f, nin=%d)", n->w[0].val, n->w[1].val, n->b->val, n->nin); /* TODO: refactor global strtable printing */
    assert(nalloc >= 0);
    assert(allocated + nalloc + 1 < STRCAP);
    allocated += nalloc + 1;

    return STRTAB + oldalloc;
}

/* pre: len(x) == n->nin (you can't assert this, though) */
/* TODO: support bias */
Value *nfwd(Neuron *n, Value *x) {
    ncheck(n);
    for (int i = 0; i < n->nin; i++)
        valcheck(x + i);

    Value *out = valfloat(0.0);
    for (int i = 0; i < n->nin; i++)
        out = valadd(out, valmul(n->w + i, x + i));
    out = valtanh(out);

    ncheck(n);
    valcheck(out);

    return out;
}

unsigned int nparams(Neuron *n, Value **ret) {
    ncheck(n);
    assert(ret != NULL);

    for (int i = 0; i < n->nin; i++)
        ret[i] = n->w + i;
    ret[n->nin] = n->b;

    return n->nin + 1; /* +1 for bias */
}

/* TODO */
Layer *lalloc(unsigned int n, unsigned int nin, unsigned int nout) {
    assert(n > 0);
    assert(nin > 0);
    assert(nout > 0);
    assert(nlay + n <= LAYCAP);

    Layer *ret = LAYTAB + nlay;
    nlay += n;

    for (int i = 0; i < n; i++) {
        ret[i].nin = nin;
        ret[i].nout = nout;
        ret[i].ns = nalloc(nout, nin);
    }

    return ret;
}

/* TODO */
void linit(Layer *l) {
    assert(l != NULL);

    for (int i = 0; i < l->nout; i++)
        ninit(l->ns + i);

    lcheck(l);
}

void lcheck(Layer *l) {
    assert(l != NULL);
    assert(l->nin > 0);
    assert(l->nout > 0);
    for (int i = 0; i < l->nout; i++)
        ncheck(l->ns + i);
}

unsigned int lfwd(Layer *l, Value *x, Value **ret) {
    lcheck(l);
    for (int i = 0; i < l->nin; i++)
        valcheck(x + i);

    for (int i = 0; i < l->nout; i++)
        ret[i] = nfwd(l->ns + i, x);

    for (int i = 0; i < l->nout; i++)
        valcheck(ret[i]);

    return l->nout;
}

unsigned int lparams(Layer *l, Value **ret) {
    lcheck(l);
    assert(ret != NULL);

    unsigned int n = 0;
    for (int i = 0; i < l->nout; i++)
        n += nparams(l->ns + i, ret + n);

    return n;
}

void mlpalloc(unsigned int nin, unsigned int *nouts, unsigned int n_nouts) {
    themlp.layers[0] = lalloc(1, nin, nouts[0]);
    for (int i = 0; i < n_nouts - 1; i++)
        themlp.layers[i] = lalloc(1, nouts[i], nouts[i + 1]);
}

void mlpassert(MLP *mlp) {
    /* */
}

void mlpinit(MLP *mlp) {
    assert(mlp != NULL);
}