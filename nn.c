#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "babel.h"

#define frand() ((double) rand() / (RAND_MAX + 1.0))

Value WTTAB[VALCAP];
unsigned int nwt;

extern char STRTAB[STRCAP];
extern unsigned int allocated;

double uniform(int lo, int hi) {
    assert(lo < hi);

    return ((hi - lo) * frand()) + lo;
}

void ncheck(Neuron *n) {
    assert(n != NULL);
    assert(n->nin > 0);
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
    ncheck(n);
    assert(nin == n->nin);
    assert(nin > 0);
    for (int i = 0; i < nin; i++)
        valcheck(x + i);

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

    ncheck(n);
    valcheck(act);
    valcheck(ret);

    return ret;
}

unsigned int nparams(Neuron *n, Value **ret) {
    ncheck(n);
    assert(ret != NULL);

    for (int i = 0; i < n->nin; i++)
        ret[i] = n->w + i;
    ret[n->nin] = &(n->b);

    return n->nin + 1;
}
