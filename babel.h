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

#define BUFSIZE 128
#define FEATSIZE 2
#define DATASIZE 100

typedef struct {
  int x;
  int y;
} Sample;

typedef enum { BINOP_ADD, BINOP_MUL } BinOpType;

typedef struct {
    BinOpType ty;
    int arg1;
    int arg2;
} BinOp;

#define ADD_BINOP(x, y) ((BinOp){BINOP_ADD, .arg1 = x, .arg2 = y})
#define MUL_BINOP(x, y) ((BinOp){BINOP_MUL, .arg1 = x, .arg2 = y})

typedef enum { AST_NUM, AST_BINOP } ASTType;

typedef struct {
    ASTType ty;
    union {
        int num;
        BinOp binop;
    } as;
} AST;

#define NUM_AST(v) ((AST){.ty = AST_NUM, {.num = v}})
#define BINOP_AST(v) ((AST){.ty = AST_BINOP, {.binop = v}})
