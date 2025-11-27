#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "babel.h"


int main(int argc, char **argv) {
    engineinit();

    printf("\n");
    printf("--- Neuron ---\n");
    unsigned int nin = 3;
    // Neuron *n = nalloc(1, 3);

    // ninit(n);

    // printf("%.5f %.5f %.5f\n", n->w[0].val, n->w[1].val, n->w[2].val); /* show weights */

    /* TODO: make it easy to construct 2d data? */

    unsigned int nepoch = 32;

    Value *xs[4] = {valfloats(3, (float[]){2.0, 3.0, -1.0}), 
                    valfloats(3, (float[]){3.0, -1.0, 0.5}),
                    valfloats(3, (float[]){0.5, 1.0, 1.0}), 
                    valfloats(3, (float[]){1.0, 1.0, -1.0})};
    Value *ys = valfloats(4, (float[]){1.0, -1.0, -1.0, 1.0});

    printf("\n --- Training neuron... ---\n");
    Value *params[VALCAP] = {0};
    Value *loss = valfloat(0.0);
    // for (int i = 0; i < nepoch; i++) {
    //     /* loss on the batch */
    //     valinit(loss, VAL_FLOAT, 0.0, NULL, NULL);
    //     for (int j = 0; j < 4; j++) {
    //         Value *ygt = ys + j;
    //         Value *ypred = nfwd(n, xs[j]);
    //         Value *l = valpow(valsub(ypred, ygt), 2);
    //         loss = valadd(loss, l);
    //     }

    //     /* zero grad */
    //     unsigned int paramslen = nparams(n, params);
    //     for (int i = 0; i < paramslen; i++)
    //         params[i]->grad = 0.0;

    //     /* backward */
    //     valbwd(loss);

    //     /* optimizer step */
    //     paramslen = nparams(n, params);
    //     for (int i = 0; i < paramslen; i++)
    //         params[i]->val += -0.1 * params[i]->grad;

    //     printf("loss: %.5f\n", loss->val);
    // }

    printf("\n --- Training MLP... ---\n");
    MLP *mlp = mlpalloc(nin, (int[]){4, 4, 1}, 3);

    mlpinit(mlp);

    memset(params, 0, sizeof(Value*) * VALCAP);
    loss = valfloat(0.0);
    for (int i = 0; i < nepoch; i++) {
        /* per-batch loss */
        valinit(loss, VAL_FLOAT, 0.0, NULL, NULL);
        for (int j = 0; j < 4; j++) {
            Value *ygt = ys + j;
            Value *ypred = mlpfwd(mlp, xs[j]);
            Value *l = valpow(valsub(ypred, ygt), 2);
            loss = valadd(loss, l);
        }
    }

    return EXIT_SUCCESS;
}
