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
/* #define STRSIZE 6 */
#define DATASIZE 100
#define MAXTOKS 2
#define ARRCAP 12

#define PROGRAM_T Expr

typedef struct {
  int x;
  int y;
} Sample;

typedef struct {
  int *xs;
  int *ys;
  unsigned int len;
  unsigned int cap;
} Samples;


typedef struct Expr Expr;
typedef struct BinOp BinOp;

typedef enum { BINOP_ADD, BINOP_MUL } BinOpType;

/* TODO: Do I want pointers here? */
struct BinOp {
    BinOpType tag;
    Expr     *e1;
    Expr     *e2;
} ;

#define ADD_BINOP(x, y) ((BinOp){BINOP_ADD, .e1 = x, .e2 = y})
#define MUL_BINOP(x, y) ((BinOp){BINOP_MUL, .e1 = x, .e2 = y})

typedef enum { EXPR_NUM, EXPR_BINOP } ExprType;

/* expression/ast datatype for the grammar */
struct Expr {
    ExprType tag;
    union {
        int    num;
        BinOp *binop;
    } as;
};

#define NUM_EXPR(v) ((Expr){.tag = EXPR_NUM, {.num = v}})
#define BINOP_EXPR(v) ((Expr){.tag = EXPR_BINOP, {.binop = v}})

typedef struct {
    Expr        *buf;
    unsigned int len;
    unsigned int cap;
} Programs;

char *bopshow(BinOp *);
char *eshow(Expr *);
