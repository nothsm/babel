#include <stdbool.h>

#define BABEL_VERSION "0.1.0"
#define BABEL_DATE "2025-10-06"
#define BABEL_LOGO                                                             \
  "▄▄                  ▄▄                  ▄▄▄▄     \n"                        \
  "██                  ██                  ▀▀██     \n"                        \
  "██▄███▄    ▄█████▄  ██▄███▄    ▄████▄     ██     \n"                        \
  "██▀  ▀██   ▀ ▄▄▄██  ██▀  ▀██  ██▄▄▄▄██    ██     \n"                        \
  "██    ██  ▄██▀▀▀██  ██    ██  ██▀▀▀▀▀▀    ██     \n"                        \
  "███▄▄██▀  ██▄▄▄███  ███▄▄██▀  ▀██▄▄▄▄█    ██▄▄▄  \n"                        \
  "▀▀ ▀▀▀     ▀▀▀▀ ▀▀  ▀▀ ▀▀▀      ▀▀▀▀▀      ▀▀▀▀  "

#define BUFSIZE 256
/* #define MEMCAP (64 * 1024) /\* 64KB mem lim *\/ */
#define MEMCAP (1024 * 1024 * 1024) /* 256MB mem lim... TODO: decrease this shit! */
#define ARRCAP 1024 /* TODO: decrease this */
#define STRCAP (1024 * 32)
#define VALCAP 2048 /* TODO: decrease this */
#define NEURCAP 128
#define LAYCAP 64
#define RECLIM 32
#define MAXPROGRAMS (32 * 1024 * 1024) /* 8MB limit... TODO: decrease this */
#define NGROW 32
#define NSHOW 100
#define SEED 546

#define PROGRAM_T Expr
#define DTYPE float

typedef struct {
  bool quiet;
  bool filter;
  unsigned int seed;
} Config;

typedef struct {
  char *buf;
  unsigned int len;
  unsigned int cap;
} Arena;

typedef struct {
  void *(*malloc)(unsigned int, void *ctx);
  void  (*free)(void *, void *ctx);
  void *(*realloc)(void *, unsigned int, void *ctx);
  void   *ctx;
} Allocator;

typedef struct {
  int *xs;
  int *ys;
  unsigned int len;
  unsigned int cap;
} Samples;

typedef struct Expr Expr;
typedef struct BinOp BinOp;
typedef struct RecOp RecOp;

typedef enum { BINOP_ADD, BINOP_MUL } BinOpType;

/* TODO: Do I want pointers here? */
struct BinOp {
    BinOpType tag;
    Expr     *e1;
    Expr     *e2;
} ;

#define ADD_BINOP(x, y) ((BinOp){BINOP_ADD, .e1 = x, .e2 = y})
#define MUL_BINOP(x, y) ((BinOp){BINOP_MUL, .e1 = x, .e2 = y})

typedef enum { EXPR_NUM, EXPR_INPUT, EXPR_REC, EXPR_BINOP } ExprType;

/* expression/ast datatype for the grammar */
struct Expr {
    ExprType tag;
    union {
        int     num;
        Expr   *rec;
        BinOp  *binop;
    } as;
};

#define NUM_EXPR(v)   ((Expr){.tag = EXPR_NUM,   {.num = v}})
#define INPUT_EXPR()  ((Expr){.tag = EXPR_INPUT, {.num = 0}})
#define REC_EXPR(v)   ((Expr){.tag = EXPR_REC,   {.rec = v}})
#define BINOP_EXPR(v) ((Expr){.tag = EXPR_BINOP, {.binop = v}})

/* --------------------------------- engine.c ---------------------------------- */

typedef struct {
    Expr        *buf;
    unsigned int len;
    unsigned int cap;
} Programs;

typedef struct Value Value;

/* TODO: add noop? */
typedef enum {
    VAL_ADD,
    VAL_MUL,
    VAL_TANH,
    VAL_FLOAT
} ValueType;

/* Can I just make this a normal Expr? */
struct Value {
    unsigned int id;
    ValueType op;
    float val;
    float grad;
    Value *prev1;
    Value *prev2;
};

void engineinit(void);

void stdbg(unsigned int, unsigned int);

Value *valinit(Value *, ValueType, float, Value *, Value *);
Value *valalloc(unsigned int n);
void valcheck(Value *);
bool valeq(Value *, Value *);
char *valshow(Value *);
char *valsexpr(Value *);
Value *valfloats(unsigned int, float *);
Value *valfloat(float);
Value *valmul(Value *, Value *);
Value *valadd(Value *, Value *);
Value *valtanh(Value *);
Value *valsub(Value *, Value *);
Value *valpow(Value *, unsigned int);
void valbwd(Value *);


/* ----------------------------------- nn.c ------------------------------------ */

typedef struct {
    Value *w; /* array of Value's of size nin */
    Value *b; /* single Value */
    unsigned int nin;
} Neuron;

typedef struct {
  Neuron *ns;
  unsigned int nin;
  unsigned int nout;
} Layer;

typedef struct {
  Layer **layers;
  unsigned int nin;
  unsigned int *nouts;
} MLP;

void ninit(Neuron *n);
Neuron *nalloc(unsigned int n, unsigned int nin);
void ncheck(Neuron *n);
char *nshow(Neuron *n);
Value *nfwd(Neuron *n, Value *x);
unsigned int nparams(Neuron *n, Value **ret);

/* TODO: Implement these */
void linit(Layer *l);
Layer *lalloc(unsigned int n, unsigned int nin, unsigned int nout);
void lcheck(Layer *l);
char *lshow(Layer *l);
unsigned int lfwd(Layer *l, Value *x, Value **ret);
unsigned int lparams(Layer *l, Value **ret);

/* TODO: Implement these */
void mlpinit(MLP *mlp);
MLP *mlpalloc(unsigned int nin, unsigned int *nouts, unsigned int n_nouts);
void mlpcheck(MLP *mlp);
void mlpfwd(MLP *mlp, Value *x, Value **ret);
unsigned int mlpparams(MLP *mlp, Value **ret);

/* typedef struct { */
/*   unsigned int *dims; */
/*   unsigned int  ndims; */
/* } Shape; */

typedef struct {
  unsigned int *shape;
  unsigned int *strides; /* TODO */
  unsigned int ndims;
  float *buf;
} Matrix;

char *bopshow(BinOp *);
char *eshow(Expr *);
