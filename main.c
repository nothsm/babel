#include <stdio.h>
#include <stdlib.h>
#include "babel.h"


int main(int argc, char **argv) {
    engineinit();

    /* printf("\n"); */
    /* printf("--- add, mul, bwd ---\n"); */

    /* Value a = {0}; */
    /* Value b = {0}; */
    /* Value c = {0}; */
    /* Value f = {0}; */

    /* valinit(&a, VAL_FLOAT, 2.0, NULL, NULL); */
    /* valinit(&b, VAL_FLOAT, -3.0, NULL, NULL); */
    /* valinit(&c, VAL_FLOAT, 10.0, NULL, NULL); */
    /* Value *e = valmul(&a, &b); */
    /* Value *d = valadd(e, &c); */
    /* valinit(&f, VAL_FLOAT, -2.0, NULL, NULL); */
    /* Value *L = valmul(d, &f); */

    /* printf("a = %s\n", valsexpr(&a)); */
    /* printf("b = %s\n", valsexpr(&b)); */
    /* printf("c = %s\n", valsexpr(&c)); */
    /* printf("e = %s\n", valsexpr(e)); */
    /* printf("d = %s\n", valsexpr(d)); */
    /* printf("f = %s\n", valsexpr(&f)); */
    /* printf("L = %s\n", valsexpr(L)); */
    /* printf("backpropagating...\n"); */
    /* valbwd(L); */
    /* printf("a = %s\n", valsexpr(&a)); */
    /* printf("b = %s\n", valsexpr(&b)); */
    /* printf("c = %s\n", valsexpr(&c)); */
    /* printf("e = %s\n", valsexpr(e)); */
    /* printf("d = %s\n", valsexpr(d)); */
    /* printf("f = %s\n", valsexpr(&f)); */
    /* printf("L = %s\n", valsexpr(L)); */

    /* printf("\n"); */
    /* printf("--- tanh ---\n"); */
    /* Value x1 = {0}; */
    /* Value x2 = {0}; */
    /* Value w1 = {0}; */
    /* Value w2 = {0}; */
    /* Value b_ = {0}; */

    /* valinit(&x1, VAL_FLOAT, 2.0, NULL, NULL); */
    /* valinit(&x2, VAL_FLOAT, 0.0, NULL, NULL); */
    /* valinit(&w1, VAL_FLOAT, -3.0, NULL, NULL); */
    /* valinit(&w2, VAL_FLOAT, 1.0, NULL, NULL); */
    /* valinit(&b_, VAL_FLOAT, 6.8813735870195432, NULL, NULL); */
    /* Value *x1w1 = valmul(&x1, &w1); */
    /* Value *x2w2 = valmul(&x2, &w2); */
    /* Value *x1w1x2w2 = valadd(x1w1, x2w2); */
    /* Value *n_ = valadd(x1w1x2w2, &b_); */
    /* Value *o = valtanh(n_); */
    /* printf("x1 = %s\n", valsexpr(&x1)); */
    /* printf("x2 = %s\n", valsexpr(&x2)); */
    /* printf("w1 = %s\n", valsexpr(&w1)); */
    /* printf("w2 = %s\n", valsexpr(&w2)); */
    /* printf("b_ = %s\n", valsexpr(&b_)); */
    /* printf("x1w1 = %s\n", valsexpr(x1w1)); */
    /* printf("x2w2 = %s\n", valsexpr(x2w2)); */
    /* printf("x1w1x2w2 = %s\n", valsexpr(x1w1x2w2)); */
    /* printf("n_ = %s\n", valsexpr(n_)); */
    /* printf("o = %s\n", valsexpr(o)); */
    /* printf("backpropagating...\n"); */
    /* valbwd(o); */
    /* printf("x1 = %s\n", valsexpr(&x1)); */
    /* printf("x2 = %s\n", valsexpr(&x2)); */
    /* printf("w1 = %s\n", valsexpr(&w1)); */
    /* printf("w2 = %s\n", valsexpr(&w2)); */
    /* printf("b_ = %s\n", valsexpr(&b_)); */
    /* printf("x1w1 = %s\n", valsexpr(x1w1)); */
    /* printf("x2w2 = %s\n", valsexpr(x2w2)); */
    /* printf("x1w1x2w2 = %s\n", valsexpr(x1w1x2w2)); */
    /* printf("n_ = %s\n", valsexpr(n_)); */
    /* printf("o = %s\n", valsexpr(o)); */


    printf("\n");
    printf("--- Neuron ---\n");
    Neuron *n = nalloc(1);
    unsigned int nin = 3;

    ninit(n, nin);
    printf("%.5f %.5f %.5f\n", n->w[0].val, n->w[1].val, n->w[2].val);

    Value x[3] = {0};
    valinit(x, VAL_FLOAT, 2.0, NULL, NULL);
    valinit(x + 1, VAL_FLOAT, 3.0, NULL, NULL);
    valinit(x + 2, VAL_FLOAT, -1.0, NULL, NULL);
    Value *vret = nfwd(n, x, nin);
    printf("%s\n", valsexpr(vret));
    valbwd(vret);
    printf("%s\n", valsexpr(vret));

    Value X0[3] = {0};
    Value X1[3] = {0};
    Value X2[3] = {0};
    Value X3[3] = {0};
    Value *xs[4] = {X0, X1, X2, X3};
    Value ys[4] = {0};
    valinit(X0 + 0, VAL_FLOAT, 2.0, NULL, NULL);
    valinit(X0 + 1 , VAL_FLOAT, 3.0, NULL, NULL);
    valinit(X0 + 2, VAL_FLOAT, -1.0, NULL, NULL);

    valinit(X1 + 0, VAL_FLOAT, 3.0, NULL, NULL);
    valinit(X1 + 1 , VAL_FLOAT, -1.0, NULL, NULL);
    valinit(X1 + 2, VAL_FLOAT, 0.5, NULL, NULL);

    valinit(X2 + 0, VAL_FLOAT, 0.5, NULL, NULL);
    valinit(X2 + 1 , VAL_FLOAT, 1.0, NULL, NULL);
    valinit(X2 + 2, VAL_FLOAT, 1.0, NULL, NULL);

    valinit(X3 + 0, VAL_FLOAT, 1.0, NULL, NULL);
    valinit(X3 + 1 , VAL_FLOAT, 1.0, NULL, NULL);
    valinit(X3 + 2, VAL_FLOAT, -1.0, NULL, NULL);

    valinit(ys + 0, VAL_FLOAT, 1.0, NULL, NULL);
    valinit(ys + 1, VAL_FLOAT, -1.0, NULL, NULL);
    valinit(ys + 2, VAL_FLOAT, -1.0, NULL, NULL);
    valinit(ys + 3, VAL_FLOAT, 1.0, NULL, NULL);

    printf("\n --- Training neuron... ---\n");
    for (int i = 0; i < 20; i++) {
        /* Value *ypred[4] = {0}; */
        Value *loss = NULL;
        for (int j = 0; j < 4; j++) {
            Value *ygt = ys + j;
            Value *yout = nfwd(n, xs[j], 3);

            Value *neg = valalloc(1);
            neg->op = VAL_FLOAT;
            neg->val = -1.0;
            neg->grad = 0.0;
            neg->prev1 = NULL;
            neg->prev2 = NULL;

            Value *negygt = valmul(ygt, neg);
            Value *diff = valadd(yout, negygt);
            Value *residual = valmul(diff, diff);

            if (j == 0)
                loss = residual;
            else
                loss = valadd(loss, residual);
        }
        Value *valbuf[VALCAP] = {0};
        unsigned int nparam = nparams(n, valbuf);
        for (int i = 0; i < nparam; i++)
            valbuf[i]->grad = 0.0;

        valbwd(loss);

        nparam = nparams(n, valbuf);
        for (int i = 0; i < nparam; i++)
            valbuf[i]->val += -0.1 * valbuf[i]->grad;

        printf("loss: %.5f\n", loss->val);
        /* TODO: fix pretty printer for loss */
    }

    return EXIT_SUCCESS;
}
