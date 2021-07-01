#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "scanner.h"

#ifndef PROGRAM
#define MAX_SYMBOLS 20
#else // Building for C program instead of Microcontroller
#define MAX_SYMBOLS 200
#endif

// Entry in the symbol table
typedef struct {
    uint32_t name;
    int16_t value;
} SymbolEntry;


// Upper 5 bit of type are the actual type, lower 3 bit contain the current position
typedef struct {
    uint8_t children[3];
    uint8_t type;
} NodeTypeBase;


// Lower 28 bit store the hash of identifier, upper 4 bit are control information
typedef struct {
    uint32_t value;
} NodeTypeId;


typedef struct {
    int16_t value;
    uint8_t _pad;
    uint8_t type;
} NodeTypeValue;

typedef struct {
    uint8_t Condition;
    uint8_t Then;
    uint8_t Else;
    uint8_t type;
} NodeTypeIf;

typedef struct {
    uint8_t Condition;
    uint8_t Stmt;
    uint8_t _pad;
    uint8_t type;
} NodeTypeWhile;

typedef enum {
    OP_NONE,
    OP_EXP = (1 << 3),
    OP_NEG = (2 << 3),
    OP_MULT= (3 << 3),
    OP_DIV,
    OP_MOD,
    OP_PLUS =(4 << 3),
    OP_MINUS,
    OP_LSHIFT=(5 << 3),
    OP_RSHIFT,
    OP_EQU = (6 << 3),
    OP_NEQ,
    OP_GT,
    OP_GE,
    OP_LT,
    OP_LE,
    OP_NOT = (7 << 3),
    OP_AND = (8 << 3),
    OP_OR =  (9 << 3),
} ExprOperands;

#define IS_EXPR_NODE(node) (GET_NODE_TYPE(node) == TYPE_EXPR || GET_NODE_TYPE(node) == TYPE_EXPR_BRACKET)
typedef struct {
    uint8_t left;
    uint8_t right;
    uint8_t op;
    uint8_t type;
} NodeTypeExpr;


#define ASSIGN_EMPTY(node) (!((node).Assign.status & (1<<7)))
#define ASSIGN_CLEAR_EMPTY(node) (node).Assign.status |= (1<<7);
typedef struct {
    uint8_t Id;
    uint8_t Expr;
    uint8_t status;
    uint8_t type;
} NodeTypeAssign;


#define STMT_GET_INDENT(node) ((node).Stmt.status >> 1)
#define STMT_SET_INDENT(node, indent) (node).Stmt.status = ((node).Stmt.status&1) | indent << 1;

#define STMT_HAS_NEXT(node) ((node).Stmt.status & 1)
#define STMT_SET_NEXT(node, next) {(node).Stmt.status |= 1; (node).Stmt.Next = next;};

// status: 7:1 -> indent, 0 -> hasNext
typedef struct {
    uint8_t status;
    uint8_t Stmt;
    uint8_t Next;
    uint8_t type;
} NodeTypeStmt;

#define FUNC_GET_ARGC(node) ((node).Func.status & 0xF)
#define FUNC_INC_ARGC(node) {(node).Func.status++;};
#define FUNC_SET_ARGC(node, argc) {(node).Func.status = (((node).Func.status & 0xF0) | (argc));};
#define FUNC_GET_SUBPOS(node) (((node).Func.status >> 4) & 3)
#define FUNC_SET_SUBPOS(node, p) {(node).Func.status = ((node).Func.status & 0xCF) | p << 4;};
// status: 3:0 argc
// status: 5:4 subpos
typedef struct {
    uint8_t status;
    uint8_t Params;
    uint8_t Body;
    uint8_t type;
} NodeTypeFunc;

#define GET_CALL_TYPE(node) ((node).Call.status >> 6)
#define SET_CALL_TYPE(node, type) {(node).Call.status = (type << 6) | ((node).Call.status & 0xC0);};
// status: 3:0 argc
// status: 5:4 subpos
// status: 7:6 special function calls
typedef struct {
    uint8_t status;
    uint8_t Args;
    uint8_t Id;
    uint8_t type;
} NodeTypeCall;
typedef enum {
    CALL_FUN,
    CALL_PUT,
    CALL_GET,
    CALL_SET
} CallTypes;

// status: Each node can store up to 2 params
typedef struct {
    uint8_t Name1;
    uint8_t Name2;
    uint8_t Next;
    uint8_t type;
} NodeTypeParam;


typedef enum {
    BUILTIN_BREAK,
    BUILTIN_CONTINUE,
    BUILTIN_RETURN,
    BUILTIN_DEL,
    BUILTIN_PRINT,
    BUILTIN_PRINTN,
} BuiltinFunctions;
#define SINGLE_ARG_OP(op) (op >= BUILTIN_RETURN)
#define IS_PRINT_OP(op) (op >= BUILTIN_PRINT)
// status: Each node can store up to 2 params
typedef struct {
    uint8_t function;
    uint8_t Arg1;
    uint8_t Arg2;
    uint8_t type;
} NodeTypeBuiltin;


typedef union {
    NodeTypeBase Base;
    NodeTypeId Id;
    NodeTypeValue Value;
    NodeTypeIf If;
    NodeTypeAssign Assign;
    NodeTypeExpr Expr;
    NodeTypeStmt Stmt;
    NodeTypeFunc Func;
    NodeTypeWhile While;
    NodeTypeParam Param;
    NodeTypeBuiltin Builtin;
    NodeTypeCall Call;
} NodeType;

typedef enum {
    TYPE_UNUSED,
    TYPE_VALUE,
    TYPE_IF,
    TYPE_BUILTIN,
    TYPE_STMT,
    TYPE_ASSIGN,
    TYPE_WHILE,
    TYPE_EXPR,
    TYPE_FUNC,
    TYPE_PARAM,
    TYPE_CALL,
    TYPE_EXPR_BRACKET,
    TYPE_ID=(1<<4)
} NodeTypeEnum;


#define GET_NODE_TYPE(node) (((node).Base.type & 0x80) ? TYPE_ID : ((node).Base.type >> 3)&0x1F)
#define GET_NODE_TYPE_IDX(node) ((GET_NODE_TYPE(node) == TYPE_ID) ? (TYPE_EXPR_BRACKET+1) : GET_NODE_TYPE(node) )
#define SET_NODE_TYPE(node, typeName) {(node).Base.type = typeName << 3;}
#define NODE_GET_POS(node) ((node).Base.type & 0b111)
#define NODE_INCR_POS(node) (node).Base.type++;
#define NODE_SET_POS(node, pos) (node).Base.type = ((node).Base.type & ~7) | pos;

#define GET_ID_HASH(node) (((node).Id.value) & 0xFFFFFF)
#define IS_ID_NODE(node) ((node).Base.type & 0x80)
#define SET_ID_NODE(node, hash) (node).Id.value = (hash) | (1UL << 31);

#define ACT_NODE nodes[activeNode]
#define CURRENT_NODE_TYPE GET_NODE_TYPE(ACT_NODE)
#define NEXT_NODE nodes[getFreeNode()]
#define COPY_NODE(src, dst) (dst).Id.value = (src).Id.value;

#define DIRECT_NODE nodes[*nodeId]
#define NODE (*n)

void consumeToken(parsedToken *token);

void interpreterInit(void *outputCharFun, void *heapStart);

#endif