#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "babel.h"

#define frand() ((double) rand() / (RAND_MAX + 1.0))

Neuron NTAB[NEURCAP];
unsigned int nn;

extern char STRTAB[STRCAP];
extern unsigned int allocated;

double uniform(int lo, int hi) {
    assert(lo < hi);

    return ((hi - lo) * frand()) + lo;
}

void nassert(Neuron *n) {
    assert(n != NULL);
    assert(n->nin > 0);
}

/* TODO: This should also allocate weights */
Neuron *nalloc(unsigned int n, unsigned int nin) {
    assert(nn + n <= NEURCAP);

    Neuron *ret = NTAB + nn;
    nn += n;

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

    nassert(n);
}

/* TODO: generalize this to > 2 input dims */
char *nshow(Neuron *n) {
    unsigned int nalloc;
    unsigned int oldalloc;
    char *s;

    assert(n->nin >= 2);
    nassert(n);

    oldalloc = allocated;

    nalloc = snprintf(STRTAB + allocated, STRCAP, "Neuron(w=[%.3f, %.3f], b=%.3f, nin=%d)", n->w[0].val, n->w[1].val, n->b->val, n->nin); /* TODO: refactor global strtable printing */
    assert(nalloc >= 0);
    assert(allocated + nalloc + 1 < STRCAP);
    allocated += nalloc + 1;

    return STRTAB + oldalloc;
}

/* last addition is put into act */
/* len(multmp) = nin */
/* len(addtmp) = nin - 1 (these bounds are wrong) */
/* TODO: support bias */
/* TODO: Fix this kludge */
Value *nfwd(Neuron *n, Value *x, unsigned int nin) {
    nassert(n);
    assert(nin == n->nin);
    assert(nin > 0);
    for (int i = 0; i < nin; i++)
        valassert(x + i);

    /* TODO: These allocate too much memory */
    Value *multmp[VALCAP] = {0};
    Value *addtmp[VALCAP] = {0};

    Value *w = n->w;

    Value *act = NULL;
    if (nin == 1) {
        assert(false);

        act = valmul(x + 0, w + 0);
    } else if (nin == 2) {
        for (int i = 0; i < nin; i++)
            multmp[i] = valmul(x + i, w + i);

        act = valadd(multmp[0], multmp[1]);
    } else {
        assert(nin >= 3);

        for (int i = 0; i < nin; i++)
            multmp[i] = valmul(x + i, w + i);

        addtmp[0] = valadd(multmp[0], multmp[1]);
        for (int i = 0; i < nin - 2; i++)
            addtmp[i + 1] = valadd(addtmp[i], multmp[i + 2]);
        act = valadd(addtmp[(nin - 2) - 1], multmp[nin - 1]);
    }
    Value *ret = valtanh(act);

    nassert(n);
    valassert(act);
    valassert(ret);

    return ret;
}

unsigned int nparams(Neuron *n, Value **ret) {
    nassert(n);
    assert(ret != NULL);

    for (int i = 0; i < n->nin; i++)
        ret[i] = n->w + i;
    ret[n->nin] = &(n->b);

    return n->nin + 1;
}

/* TODO */
void linit(Layer *l, unsigned int nin, unsigned int nout) {
    assert(l != NULL);
    assert(nin > 0);
    assert(nout > 0);

    Neuron *ns = nalloc(nout, nin);
    for (int i = 0; i < nout; i++)
        ninit(ns + i);

    l->nin = nin;
    l->nout = nout;
    l->ns = ns;

    lassert(l);
}

void lassert(Layer *l) {
    assert(l != NULL);
    assert(l->nin > 0);
    assert(l->nout > 0);
    for (int i = 0; i < l->nout; i++)
        nassert(l->ns + i);
}

unsigned int lfwd(Layer *l, Value *x, Value **ret) {
    lassert(l);
    for (int i = 0; i < l->nin; i++)
        valassert(x + i);

    int i;
    for (i = 0; i < l->nout; i++)
        ret[i] = nfwd(l->ns + i, x, l->nin);

    for (int j = 0; j < i; j++)
        valassert(ret[j]);

    return i;
}

unsigned int lparams(Layer *l, Value **ret) {
    lassert(l);
    assert(ret != NULL);

    unsigned int n = 0;
    for (int i = 0; i < l->nout; i++)
        n += nparams(l->ns + i, ret + n);

    return n;
}