#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "babel.h"

#define frand() ((double) rand() / (RAND_MAX + 1.0))

Neuron NEURTAB[NEURCAP];
unsigned int nneur;

Layer LAYTAB[LAYCAP];
unsigned int nlay;

extern char STRTAB[STRCAP];
extern unsigned int allocated;

MLP themlp = {0};
Layer *themlp_layers[LAYCAP] = {0};

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
        valinit(n->w + i, VAL_FLOAT, (float)uniform(-1, 1), NULL, NULL);

    valinit(n->b, VAL_FLOAT, (float)uniform(-1, 1), NULL, NULL);

    ncheck(n);
}

/* TODO: generalize this to > 2 input dims */
char *nshow(Neuron *n) {
    int nalloc;
    unsigned int unsigned_nalloc;
    unsigned int oldalloc;

    assert(n->nin >= 2);
    ncheck(n);

    oldalloc = allocated;

    nalloc = snprintf(STRTAB + allocated, STRCAP, "Neuron(w=[%.3f, %.3f], b=%.3f, nin=%d)", n->w[0].val, n->w[1].val, n->b->val, n->nin); /* TODO: refactor global strtable printing */
    assert(nalloc >= 0);

    unsigned_nalloc = (unsigned int)nalloc;

    assert(allocated + unsigned_nalloc + 1 < STRCAP);
    allocated += unsigned_nalloc + 1;

    return STRTAB + oldalloc;
}

/* pre: len(x) == n->nin (you can't assert this, though) */
/* TODO: support bias */
/* TODO: Should take in a Value** */
Value *nfwd(Neuron *n, Value **x) {
    ncheck(n);
    for (int i = 0; i < n->nin; i++)
        valcheck(x[i]);

    Value *out = valfloat(0.0);
    for (int i = 0; i < n->nin; i++)
        out = valadd(out, valmul(n->w + i, x[i]));
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

/* TODO: This should take in a Value** */
unsigned int lfwd(Layer *l, Value **x, Value **ret) {
    lcheck(l);
    for (int i = 0; i < l->nin; i++)
        valcheck(x[i]);

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

MLP *mlpalloc(unsigned int nin, unsigned int *nouts, unsigned int n_nouts) {
    assert(nin > 0);
    assert(nouts != NULL);
    for (int i = 0; i < n_nouts; i++)
        assert(nouts[i] > 0);
    assert(n_nouts > 0);

    memset(&themlp_layers, 0, sizeof(Layer*) * LAYCAP);
    themlp.layers = themlp_layers;

    themlp.layers[0] = lalloc(1, nin, nouts[0]); 
    for (int i = 0; i < n_nouts - 1; i++)
        themlp.layers[i + 1] = lalloc(1, nouts[i], nouts[i + 1]);  /* i + 1 since we already set layers[0] */

    themlp.nin = nin;
    themlp.nlayers = n_nouts;
    /* TODO: Save nouts in themlp */

    for (int i = 0; i < themlp.nlayers; i++)
        lcheck(themlp.layers[i]);

    return &themlp;
}

void mlpcheck(MLP *mlp) {
    assert(mlp != NULL);
    assert(mlp->nin > 0);
    for (int i = 0; i < mlp->nlayers; i++)
        lcheck(mlp->layers[i]);
}

void mlpinit(MLP *mlp) {
    assert(mlp != NULL);

    for (int i = 0; i < mlp->nlayers; i++)
        linit(mlp->layers[i]);

    mlpcheck(mlp);
}

/* Make this not a hack */
/* TODO: Fix the memory leaks in this function */
Value **mlpfwd(MLP *mlp, Value **x) {
    mlpcheck(mlp);
    assert(mlp->layers[mlp->nlayers - 1]->nout == 1);

    Value **xtmp[LAYCAP] = {0};
    xtmp[0] = x;
    for (int i = 1; i < mlp->nlayers + 1; i++)
        xtmp[i] = calloc(mlp->layers[i - 1]->nin, sizeof(Value*)); /* TODO: Don't use calloc. Use custom allocator */
 
    for (int i = 0; i < mlp->nlayers; i++)
        lfwd(mlp->layers[i], xtmp[i], xtmp[i + 1]);

    Value **out = xtmp[mlp->nlayers];

    for (int i = 0; i < mlp->layers[mlp->nlayers - 1]->nout; i++)
        valcheck(out[i]);

    return out;
}

unsigned int mlpparams(MLP *mlp, Value **ret) {
    mlpcheck(mlp);
    assert(ret != NULL);

    unsigned int n = 0;
    for (int i = 0; i < mlp->nlayers; i++)
        n += lparams(mlp->layers[i], ret + n);

    mlpcheck(mlp);

    return n;
}