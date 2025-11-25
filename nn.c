#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "babel.h"

#define frand() ((double) rand() / (RAND_MAX + 1.0))

Value WTTAB[VALCAP];
unsigned int nwt;

Neuron NTAB[VALCAP];
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

Neuron *nalloc(unsigned int n) {
    assert(nn + n < VALCAP);

    Neuron *ret = NTAB + nn;
    nn += n;

    return ret;
}

void ninit(Neuron *n, unsigned int nin) {
    Value *w;
    Value b;
    int i;

    assert(nwt + nin <= VALCAP);

    w = WTTAB + nwt;
    valinit(&b, VAL_FLOAT, uniform(-1, 1), NULL, NULL);

    for (i = nwt; i < nwt + nin; i++)
        valinit(WTTAB + i, VAL_FLOAT, uniform(-1, 1), NULL, NULL);
    nwt = i;

    n->w = w;
    n->b = b;
    n->nin = nin;

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

    nalloc = snprintf(STRTAB + allocated, STRCAP, "Neuron(w=[%.3f, %.3f], b=%.3f, nin=%d)", n->w[0].val, n->w[1].val, n->b.val, n->nin); /* TODO: refactor global strtable printing */
    assert(nalloc >= 0);
    assert(allocated + nalloc + 1 < STRCAP);
    allocated += nalloc + 1;

    return STRTAB + oldalloc;
}

/* last addition is put into act */
/* len(multmp) = nin */
/* len(addtmp) = nin - 1 (these bounds are wrong) */
/* TODO: support bias */
Value *nfwd(Neuron *n, Value *x, unsigned int nin) {
    nassert(n);
    assert(nin == n->nin);
    assert(nin > 0);
    for (int i = 0; i < nin; i++)
        valassert(x + i);

    /* TODO: These take up wayyyyy too much space */
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

    Neuron *ns = nalloc(nout);
    for (int i = 0; i < nout; i++)
        ninit(ns + i, nin);

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

Value **lfwd(Layer *l, Value *x) {
    lassert(l);
    for (int i = 0; i < l->nin; i++)
        valassert(x + i);

    for (int i = 0; i < l->nout; i++)
        nfwd(l->ns + i, x, l->nin);

    return NULL;
}