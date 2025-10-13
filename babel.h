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
#define MEMCAP (512 * 1024 * 1024) /* 128MB mem lim... TODO: decrease this shit! */
#define ARRCAP 1024 /* TODO: decrease this */
#define RECLIM 32
#define MAXPROGRAMS (8 * 1024 * 1024) /* 2MB limit... TODO: decrease this */
#define NSHOW 100

#define PROGRAM_T Expr

typedef struct {
  char *buf;
  unsigned int len;
  unsigned int cap;
} Arena;

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

char *bopshow(BinOp *);
char *eshow(Expr *);
