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
#define STRCAP (1024 * 16)
#define VALCAP 128
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

typedef struct {
    Expr        *buf;
    unsigned int len;
    unsigned int cap;
} Programs;

typedef struct Value Value;

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

Value *valinit(Value *v, ValueType op, float val, Value *prev1, Value *prev2);
void valcheck(Value *);
char *valshow(Value *);
Value *valmul(Value *x, Value *y);
Value *valadd(Value *x, Value *y);
Value *valtanh(Value *x);

typedef struct {
    Value *w;
    Value b;
    unsigned int nin;
} Neuron;

void ncheck(Neuron *n);
void ninit(Neuron *n, unsigned int nin);
char *nshow(Neuron *n);
Value *nfwd(Neuron *n, Value *x, unsigned int nin);
unsigned int nparams(Neuron *n, Value **ret);

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

typedef struct {
  unsigned int foo;
} BanditEnv;

typedef struct {
  unsigned int *actions[2]; /* list of pairs */
  unsigned int nactions;
  unsigned int *xmoves[2];
  unsigned int nxmoves;
  unsigned int *omoves[2];
  unsigned int nomoves;
  unsigned int step;
  unsigned int sample;
  unsigned int reset;
  unsigned int makenew;
} TicEnv;

typedef struct {
  unsigned int foo;
} TSPEnv;

typedef struct {
  unsigned int foo;
} SudokuEnv;

typedef struct {
  unsigned int foo;
} SynthesisEnv;

typedef struct {
  void *env;
  unsigned int get_action;
  unsigned int update;
} EpsGreedyAgent;

typedef struct {
  unsigned int foo;
} UCBAgent;

typedef struct {
  unsigned int foo;
} GradBanditAgent;

typedef struct {
  unsigned int foo;
} MCTSAgent;

/*
 * - simulated annealing
 * - evolutionary algorithms
 * - NEAT
 * - genetic algorithms
 */

char *bopshow(BinOp *);
char *eshow(Expr *);
