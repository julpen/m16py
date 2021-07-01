#include "scanner.h"
#include "interpreter.h"
#include "interpreter_strings.h"
#include <stdbool.h>
#include <string.h>
#include <crc/crc.h>
#include <stdlib.h>


SymbolEntry symbolTable[MAX_SYMBOLS];

NodeType* nodes;

uint8_t nodeCount = 1; // Highest node that has been initialized
uint8_t tmpId = 0;
uint8_t activeNode = 0;

uint8_t interpreterStatus = 0;

#define SET_RETURN()   {interpreterStatus|=1;}
#define CLEAR_RETURN() {interpreterStatus&=~1;}
#define GOT_RETURN()   (interpreterStatus&1)
#define SET_BREAK()   {interpreterStatus|=(1<<1);}
#define CLEAR_BREAK() {interpreterStatus&=~(1<<1);}
#define GOT_BREAK()   (interpreterStatus&(1<<1))


int8_t idToOp(keyword id);
int16_t evaluateExpression(NodeType *node);
bool isValidExpression(NodeType *node);
bool isValidStatement(NodeType *node);
void finishIndent(NodeType *node);
void freeNode(NodeType *node);
uint8_t getFreeNode();
int16_t evaluateNode(NodeType *node);
void handleTokenNode(uint8_t *nodeId, parsedToken *token);
void handleTokenAssign(uint8_t *nodeId, parsedToken *token);
void handleTokenIdValue(uint8_t *nodeId, parsedToken *token);
void handleTokenUnused(uint8_t *nodeId, parsedToken *token);
void handleTokenExpr(uint8_t *nodeId, parsedToken *token, uint8_t *rootPtr);
void handleTokenIf(uint8_t *nodeId, parsedToken *token);
void handleTokenWhile(uint8_t *nodeId, parsedToken *token);
void handleTokenStmt(uint8_t *nodeId, parsedToken *token);
void handleTokenFunc(uint8_t *nodeId, parsedToken *token);
void handleTokenBuiltin(uint8_t *nodeId, parsedToken *token);
void handleTokenCall(uint8_t *nodeId, parsedToken *token);
void transformValIdToExpr(uint8_t *nodeId, ExprOperands op);

int8_t getIndentLevel(NodeType *node);

#define GET_PARENT(_n) ((NodeType *) getParentNode(_n, false))
#define GET_PARENT_PTR(_n) ((uint8_t *) getParentNode(_n, true))
void *getParentNode(NodeType *node, bool ptr);

#include "helpers/output.c"
#include "helpers/node.c"
#include "helpers/symbol_table.c"
#include "helpers/expr.c"


void resetInterpreter() {
    int i;
    for (i=0; i < nodeCount; i++) {
        nodes[i].Id.value = 0;
    }
    for (i=0; i < MAX_SYMBOLS; i++) {
        symbolTable[i].name = 0;
        symbolTable[i].value = 0;
    }
    activeNode = 0;

}

void consumeToken(parsedToken *token) {
    if (token->token == ID_DEBUG) {
        printDebugInfo();
        return;
    }
    if (token->token == ID_YEET) {
        resetInterpreter();
        return;
    }
    if (token->token == ID_EOL || token->token == ID_SEMICOLON) {
        switch((NodeTypeEnum) CURRENT_NODE_TYPE) {
            case TYPE_UNUSED:
                return;
            case TYPE_ID:
            case TYPE_VALUE:
            case TYPE_EXPR_BRACKET:
            case TYPE_EXPR:
                outputValue(evaluateNode(&ACT_NODE), true);
                freeNode(&ACT_NODE);
                return;
            case TYPE_CALL:
                evaluateNode(&ACT_NODE);
                freeNode(&ACT_NODE);
                return;
            case TYPE_BUILTIN:
            case TYPE_ASSIGN:
                evaluateNode(&ACT_NODE);
                freeNode(&ACT_NODE);
                return;
            case TYPE_IF:
            case TYPE_WHILE:
            case TYPE_FUNC:
                break;
            default:
                outputMsg(str_err, str_err_consume_eol, GET_NODE_TYPE_IDX(nodes[activeNode]));
        }
    }

    handleTokenNode(&activeNode, token);

    if (CURRENT_NODE_TYPE == TYPE_IF) {
        if (NODE_GET_POS(ACT_NODE) > 5) { // If statement has been finished
            evaluateNode(&ACT_NODE);
            freeNode(&ACT_NODE);
            handleTokenNode(&activeNode, token);
        }
    }
    else if (CURRENT_NODE_TYPE == TYPE_WHILE) {
        if (NODE_GET_POS(ACT_NODE) == 4) {
            evaluateNode(&ACT_NODE);
            freeNode(&ACT_NODE);
            handleTokenNode(&activeNode, token);
        }
    }
    else if (CURRENT_NODE_TYPE == TYPE_FUNC) {
        if (NODE_GET_POS(ACT_NODE) == 7) { // Function is finished, create a new active node and expect new code
            activeNode = getFreeNode();
            handleTokenNode(&activeNode, token);
        }
    }
}

void handleTokenNode(uint8_t *nodeId, parsedToken *token) {
    switch((NodeTypeEnum) GET_NODE_TYPE(DIRECT_NODE)) {
        case TYPE_UNUSED:
            handleTokenUnused(nodeId, token);
            return;
        case TYPE_VALUE:
        case TYPE_ID:
            handleTokenIdValue(nodeId, token);
            return;
        case TYPE_ASSIGN:
            handleTokenAssign(nodeId, token);
            return;

        case TYPE_EXPR:
        case TYPE_EXPR_BRACKET:
            handleTokenExpr(nodeId, token, nodeId);
            return;

        case TYPE_IF:
            handleTokenIf(nodeId, token);
            return;

        case TYPE_WHILE:
            handleTokenWhile(nodeId, token);
            return;

        case TYPE_STMT:
            handleTokenStmt(nodeId, token);
            return;

        case TYPE_FUNC:
            handleTokenFunc(nodeId, token);
            return;

        case TYPE_BUILTIN:
            handleTokenBuiltin(nodeId, token);
            return;

        case TYPE_CALL:
            handleTokenCall(nodeId, token);
            return;

        case TYPE_PARAM: // Go to error message, handled directly as part of handleTokenFunc and should be unreachable
            break;
    }
    outputMsg(str_err, str_err_consume, GET_NODE_TYPE_IDX(nodes[*nodeId]));
}

void interpreterInit(void *outputCharFun, void *heapStart) {
    outputChar = outputCharFun;
    nodes = heapStart;
    nodes[0].Id.value = 0;
}

void handleTokenUnused(uint8_t *nodeId, parsedToken *token) {
    NodeType *n = &DIRECT_NODE;
    switch(token->token) {
        case ID_NAME:
            SET_ID_NODE(NODE, token->tokenValue);
            break;
        case ID_NUM:
            SET_NODE_TYPE(NODE, TYPE_VALUE);
            NODE.Value.value = token->tokenValue;
            break;
        case ID_MINUS:
            SET_NODE_TYPE(NODE, TYPE_EXPR);
            NODE.Expr.op = OP_NEG;
            NODE_SET_POS(NODE, 3);
            break;
        case ID_PLUS:
            SET_NODE_TYPE(NODE, TYPE_EXPR);
            NODE.Expr.op = OP_NONE;
            break;
        case ID_NOT:
            SET_NODE_TYPE(NODE, TYPE_EXPR);
            NODE.Expr.op = OP_NOT;
            NODE_SET_POS(NODE, 3);
            break;
        case ID_OPEN_ROUND:
            SET_NODE_TYPE(NODE, TYPE_EXPR_BRACKET);
            break;
        case ID_IF:
            SET_NODE_TYPE(NODE, TYPE_IF);
            break;
        case ID_WHILE:
            SET_NODE_TYPE(NODE, TYPE_WHILE);
            break;
        case ID_DEF:
            SET_NODE_TYPE(NODE, TYPE_FUNC);
            break;
        case ID_RETURN:
            SET_NODE_TYPE(NODE, TYPE_BUILTIN);
            NODE.Builtin.function = BUILTIN_RETURN;
            break;
        case ID_PRINT:
            SET_NODE_TYPE(NODE, TYPE_BUILTIN);
            NODE.Builtin.function = BUILTIN_PRINT;
            break;
        case ID_PRINTN:
            SET_NODE_TYPE(NODE, TYPE_BUILTIN);
            NODE.Builtin.function = BUILTIN_PRINTN;
            break;
        case ID_GET:
            SET_NODE_TYPE(NODE, TYPE_CALL);
            NODE_SET_POS(NODE, 3);
            SET_CALL_TYPE(NODE, CALL_GET);
            break;
        case ID_SET:
            SET_NODE_TYPE(NODE, TYPE_CALL);
            NODE_SET_POS(NODE, 3);
            SET_CALL_TYPE(NODE, CALL_SET);
            break;
        case ID_PUT:
            SET_NODE_TYPE(NODE, TYPE_CALL);
            NODE_SET_POS(NODE, 3);
            SET_CALL_TYPE(NODE, CALL_PUT);
            break;
        case ID_BREAK:
            SET_NODE_TYPE(NODE, TYPE_BUILTIN);
            NODE.Builtin.function = BUILTIN_BREAK;
            break;

        default:
            sevenSegmentPrintHex(0xE6);
    }
}


void handleTokenAssign(uint8_t *nodeId, parsedToken *token) {
    NodeType *node = &DIRECT_NODE;
    if (ASSIGN_EMPTY(*node)) {
        switch(token->token) {
            case ID_NUM:
                ASSIGN_CLEAR_EMPTY(*node);
                node->Assign.Expr = getFreeNode();
                SET_NODE_TYPE(nodes[node->Assign.Expr], TYPE_VALUE);
                nodes[node->Assign.Expr].Value.value = token->tokenValue;
                break;
            case ID_NAME:
                ASSIGN_CLEAR_EMPTY(*node);
                node->Assign.Expr = getFreeNode();
                SET_ID_NODE(nodes[node->Assign.Expr], token->tokenValue);
                break;
            case ID_PLUS:
                break;
            case ID_MINUS:
            case ID_OPEN_ROUND:
                ASSIGN_CLEAR_EMPTY(*node);
                node->Assign.Expr = getFreeNode();
                SET_NODE_TYPE(nodes[node->Assign.Expr], TYPE_EXPR);
                handleTokenExpr(&(node->Assign.Expr), token, &(node->Assign.Expr));
                break;
            default:
                sevenSegmentPrintHex(0xE7);
        }
    }
    else {
        handleTokenNode(&(node->Assign.Expr), token);
    }
}

void handleTokenWhile(uint8_t *nodeId, parsedToken *token) {
    NodeType *n = &DIRECT_NODE;
    switch(NODE_GET_POS(NODE)) {
        case 0: // Only the while keyword has been detected and we are receiving the first element of the condition
            NODE.While.Condition = getFreeNode();
            handleTokenNode(&NODE.If.Condition, token);
            NODE_INCR_POS(NODE);
            break;
        case 1: // Parsing condition
            if (token->token == ID_COLON) {
                if (!isValidExpression(&nodes[NODE.While.Condition])) {
                    outputMsg(str_err, str_err_incomplete_expr, -1);
                    return;
                }
                NODE_INCR_POS(NODE);
            }
            else {
                handleTokenNode(&NODE.If.Condition, token);
            }
            break;
        case 2: // Waiting for EOL
            if (token->token == ID_EOL) {
                NODE.While.Stmt = getFreeNode();
                SET_NODE_TYPE(nodes[NODE.While.Stmt], TYPE_STMT);
                NODE_INCR_POS(NODE);
            }
            else {
                outputMsg(str_err, str_err_expect_newline, -1);
                return;
            }
            break;
        case 3: // Parsing loop body
            if (token->indent > getIndentLevel(&NODE)) { // Indented -> pass on
                handleTokenStmt(&NODE.While.Stmt, token);
            }
            else if (token->indent == getIndentLevel(&NODE)) { // Same level -> Either we get else or we end the If statement
                if (NODE_GET_POS(nodes[NODE.While.Stmt]) == 0) {
                    outputMsg(str_err, str_err_missing_stmts, -1);
                }
                else if (token->token != ID_EOL) {
                    if (!isValidStatement(&nodes[NODE.While.Stmt])) {
                        outputMsg(str_err, str_err_incomplete_stmts, -1);
                    }
                    finishIndent(&nodes[NODE.While.Stmt]);
                    NODE_INCR_POS(NODE);
                }
            }
            else {
                outputMsg(str_err, str_err_wrong_indent, GET_NODE_TYPE_IDX(NODE));
            }
            break;
    }
}


void handleTokenIf(uint8_t *nodeId, parsedToken *token) {
    NodeType *n = &DIRECT_NODE;
    if (token->indent < getIndentLevel(&NODE)) {
        if (token->token == ID_EOL) { // Empty line is ignored
            return;
        }
        outputMsg(str_err, str_err_wrong_indent, GET_NODE_TYPE_IDX(NODE));
        return;
    }
    switch(NODE_GET_POS(NODE)) {
        case 0: // Only the if keyword has been detected and we received the first element of the condition
            NODE.If.Condition = getFreeNode();
            handleTokenNode(&NODE.If.Condition, token);
            NODE_INCR_POS(NODE);
            break;
        case 1: // The condition is being parsed
            if (token->token == ID_COLON) {
                if (!isValidExpression(&nodes[NODE.If.Condition])) {
                    outputMsg(str_err, str_err_incomplete_expr, -1);
                    return;
                }
                NODE_INCR_POS(NODE);
            }
            else {
                handleTokenNode(&NODE.If.Condition, token);
            }
            break;
        case 2: // if Condition: -> waiting for EOL
            if (token->token == ID_EOL) {
                NODE.If.Then = getFreeNode();
                SET_NODE_TYPE(nodes[NODE.If.Then], TYPE_STMT);
                NODE_INCR_POS(NODE);
            }
            else {
                outputMsg(str_err, str_err_expect_newline, -1);
                return;
            }
            break;
        case 3: // if Condition: \n -> parsing the Then
            if (token->indent > getIndentLevel(&NODE)) { // Indented -> pass on
                handleTokenStmt(&NODE.If.Then, token);
            }
            else if (token->indent == getIndentLevel(&NODE)) { // Same level -> Either we get else or we end the If statement
                if (token->token == ID_EOL) { // Empty line
                    return;
                }
                else if (NODE_GET_POS(nodes[NODE.If.Then]) == 0) {
                    outputMsg(str_err, str_err_missing_stmts, -1);
                }
                else {
                    if (!isValidStatement(&nodes[NODE.If.Then])) {
                        outputMsg(str_err, str_err_incomplete_stmts, -1);
                    }
                    if (token->token == ID_ELSE) {
                        NODE_INCR_POS(NODE);
                    } else {
                        finishIndent(&nodes[NODE.If.Then]);
                        NODE_SET_POS(NODE, 6); // Finished If without Else
                    }
                }
            }
            else {
                outputMsg(str_err, str_err_wrong_indent, GET_NODE_TYPE_IDX(NODE));
            }
            break;
        case 4: // Got else, waiting for : and \n
            if (GET_NODE_TYPE(nodes[NODE.If.Else]) == TYPE_STMT && NODE_GET_POS(nodes[NODE.If.Else]) == 0) { // Waiting for EOL
                if (token ->token == ID_EOL) {
                    NODE_INCR_POS(NODE);
                    break;
                }
                outputMsg(str_err, str_err_expect_newline, -1);
                break;
            }
            else { // Waiting for :
                if (token -> token == ID_COLON) {
                    NODE.If.Else = getFreeNode();
                    SET_NODE_TYPE(nodes[NODE.If.Else], TYPE_STMT);
                    break;
                }
                outputMsg(str_err, str_err_expect_colon, -1);
                break;
            } // TODO: Implement else if
        case 5: // Parsing the Else statement(s)
            if (token->indent > getIndentLevel(&NODE)) { // Indented -> pass on
                handleTokenStmt(&NODE.If.Else, token);
            }
            else if (token->indent == getIndentLevel(&NODE)) { // Same level -> Either we get else or we end the If statement
                if (token->token == ID_EOL) { // Empty line
                    return;
                }
                else if (NODE_GET_POS(nodes[NODE.If.Else]) == 0) {
                    outputMsg(str_err, str_err_missing_stmts, -1);
                }
                else {
                    // Need to double check if statement is complete
                    if (!isValidStatement(&nodes[NODE.If.Else])) {
                        outputMsg(str_err, str_err_incomplete_stmts, -1);
                    }
                    finishIndent(&nodes[NODE.If.Else]);
                    NODE_SET_POS(NODE, 7); // Finished If with Else
                }
            }
            break;
    }
}

#define THIS_STMT nodes[NODE.Stmt.Stmt]
void handleTokenStmt(uint8_t *nodeId, parsedToken *token) {
    NodeType *n = &DIRECT_NODE;
    if (token->indent < getIndentLevel(&NODE)) {
        if (token->token == ID_EOL) { // Empty line is ignored
            return;
        }
        outputMsg(str_err, str_err_wrong_indent, GET_NODE_TYPE_IDX(NODE));
        return;
    }
    switch(NODE_GET_POS(NODE)) {
        case 0:
            STMT_SET_INDENT(NODE, token->indent);
            NODE_INCR_POS(NODE);
            NODE.Stmt.Stmt = getFreeNode();
            handleTokenUnused(&NODE.Stmt.Stmt, token);
            break;
        case 1:
            if (token->indent > getIndentLevel(&NODE)) { // Higher indent level -> pass on
                handleTokenNode(&NODE.Stmt.Stmt, token);
                break;
            }
            else {
                if (!isValidStatement(&THIS_STMT)) {
                    handleTokenNode(&NODE.Stmt.Stmt, token);
                    break;
                }
                else { // Already a valid statement
                    if (GET_NODE_TYPE(THIS_STMT) == TYPE_IF) {
                        if (NODE_GET_POS(THIS_STMT) == 3 && token->token == ID_ELSE) { // Else corresponding to If
                            handleTokenNode(&NODE.Stmt.Stmt, token);
                            break;
                        }
                        if (token->token != ID_EOL) { // Intentional fallthrough
                            finishIndent(&nodes[NODE.Stmt.Stmt]);
                            NODE_INCR_POS(NODE);
                        }
                        else {
                            break;
                        }
                    }
                    else if (GET_NODE_TYPE(THIS_STMT) == TYPE_WHILE) { // While loop ending
                        if (token->token != ID_EOL) { // Intentional fallthrough
                            finishIndent(&nodes[NODE.Stmt.Stmt]);
                            NODE_INCR_POS(NODE);
                        }
                        else {
                            break;
                        }
                    }
                    else { // All other statements are just continued
                        if (token->token == ID_EOL || token->token == ID_SEMICOLON) {
                            NODE_INCR_POS(NODE);
                        }
                        else {
                            handleTokenNode(&NODE.Stmt.Stmt, token);
                        }
                        break;
                    }
                }
            }
        case 2:
            if (token->token == ID_EOL) {
                return;
            }
            if (token->indent > getIndentLevel(&NODE)) {
                outputMsg(str_err, str_err_wrong_indent, -1);
            }
            else {
                STMT_SET_NEXT(NODE, getFreeNode());
                SET_NODE_TYPE(nodes[NODE.Stmt.Next], TYPE_STMT);
                handleTokenNode(&NODE.Stmt.Next, token);
                NODE_INCR_POS(NODE);
            }
            break;
        case 3:
            handleTokenNode(&NODE.Stmt.Next, token);
    }
}

// Args and Params in Call/Func are at same position, so this can be used for both
void setNewArg(NodeType *n, parsedToken *token) {
    if (FUNC_GET_ARGC(*n) == 0) {
        n->Call.Args = getFreeNode();
        SET_NODE_TYPE(nodes[n->Call.Args], TYPE_PARAM);
        nodes[n->Call.Args].Param.Name1 = getFreeNode();
        handleTokenNode(&nodes[n->Call.Args].Param.Name1, token);
        FUNC_INC_ARGC(*n);
    } else {
        tmpId = n->Call.Args;
        while (NODE_GET_POS(nodes[tmpId]) == 2) {
            tmpId = nodes[tmpId].Param.Next;
        }

        if (NODE_GET_POS(nodes[tmpId]) == 0) { // One parameter stored
            NODE_SET_POS(nodes[tmpId], 1);
            nodes[tmpId].Param.Name2 = getFreeNode();
            handleTokenNode(&nodes[tmpId].Param.Name2, token);
        }
        else { // Already has 2 parameters
            NODE_SET_POS(nodes[tmpId], 2);
            nodes[tmpId].Param.Next = getFreeNode();

            tmpId = nodes[tmpId].Param.Next;
            SET_NODE_TYPE(nodes[tmpId], TYPE_PARAM);
            nodes[tmpId].Param.Name1 = getFreeNode();
            handleTokenNode(&nodes[tmpId].Param.Name1, token);
        }
        FUNC_INC_ARGC(*n);
    }
}

uint8_t* getArg(NodeType *n, uint8_t idx) {
    tmpId = n->Call.Args;
    for (uint8_t i = 0; (i+1) < idx; i=i+2) {
        tmpId = nodes[tmpId].Param.Next;
    }
    if ((idx % 2) == 0) {
        return &nodes[tmpId].Param.Name1;
    }
    return &nodes[tmpId].Param.Name2;
}

uint8_t* getCurArg(NodeType *n) {
    return getArg(n, FUNC_GET_ARGC(*n)-1);
}


void handleTokenCall(uint8_t *nodeId, parsedToken *token) {
    NodeType *n = &DIRECT_NODE;
    uint8_t *arg;
    switch(NODE_GET_POS(NODE)) {
        case 0: // Expect first argument or closing bracket
            if (token->token == ID_CLOSE_ROUND) {
                NODE_SET_POS(NODE, 7);
                return;
            }
        case 1: // Expect argument
            setNewArg(n, token);
            NODE_SET_POS(NODE, 2);
            return;
        case 2: // Parsing an argument
            arg = getCurArg(n);
            if (!isValidExpression(&nodes[*arg])) {
                handleTokenNode(arg, token);
            }
            else if (token->token == ID_COMMA) {
                NODE_SET_POS(NODE, 1);
            }
            else if (token->token == ID_CLOSE_ROUND) {
                NODE_SET_POS(NODE, 7);
            }
            else { // Pass on to argument
                handleTokenNode(arg, token);
            }
            return;
        case 3: // Special initialization phase for builtin functions
            if (token->token == ID_OPEN_ROUND) {
                NODE_SET_POS(NODE, 0);
                return;
            }
            break;
        case 7:
            if (idToOp(token->token) >= 0) {
                transformValIdToExpr(nodeId, idToOp(token->token));
                return;
            }
    }
}


void handleTokenFunc(uint8_t *nodeId, parsedToken *token) {
    NodeType *n = &DIRECT_NODE;
    switch(NODE_GET_POS(NODE)) {
        case 0: // Expect function identifier
            if (token->token != ID_NAME) {
                outputMsg(str_err, str_err_expect_id, -1);
                return;
            }
            else {
                createFunctionEntry(token->tokenValue, *nodeId);
                NODE_INCR_POS(NODE);
            }
            break;
        case 1: // Expect opening bracket
            if (token->token != ID_OPEN_ROUND) {
                outputMsg(str_err, str_err_expect_bracket, -1);
                return;
            }
            else {
                FUNC_SET_ARGC(NODE, 0);
                NODE_INCR_POS(NODE);
            }
            break;
        case 2: // Expecting first parameter or closing bracket
            if (token->token == ID_CLOSE_ROUND) {
                NODE_SET_POS(NODE, 5);
                break;
            }
        case 3: // Expecting parameter
            if (token->token == ID_NAME) {
                setNewArg(n, token);
                NODE_SET_POS(NODE, 4);
            }
            else {
                outputMsg(str_err, str_err_expect_id, -1);
                return;
            }
            break;
        case 4: // Expecting , or )
            if (token->token == ID_COMMA) { // Wait for next parameter
                NODE_SET_POS(NODE, 3);
            }
            else if (token->token == ID_CLOSE_ROUND) { // parameter list is finished
                NODE_INCR_POS(NODE);
                FUNC_SET_SUBPOS(NODE, 0);
            }
            else {
                outputMsg(str_err, str_err_expect_bracket, -1);
                return;
            }
            break;
        case 5: // Expecting : and EOL
            if (FUNC_GET_SUBPOS(NODE) == 0) {
                if (token->token == ID_COLON) {
                    FUNC_SET_SUBPOS(NODE, 1);
                    break;
                }
                else {
                    outputMsg(str_err, str_err_expect_colon, -1);
                    return;
                }
            }
            if (FUNC_GET_SUBPOS(NODE) == 1) {
                if (token->token == ID_EOL) {
                    NODE_INCR_POS(NODE);
                    NODE.Func.Body = getFreeNode();
                    SET_NODE_TYPE(nodes[NODE.Func.Body], TYPE_STMT);
                    break;
                }
            }
            break;
        case 6: // Expecting statements
            if (token->indent > getIndentLevel(&NODE)) { // Indented -> pass on
                handleTokenStmt(&NODE.Func.Body, token);
            }
            else if (token->indent == getIndentLevel(&NODE)) { // Same level -> Either we get else or we end the If statement
                if (token->token == ID_EOL) { // Empty line
                    return;
                }
                else if (NODE_GET_POS(nodes[NODE.Func.Body]) == 0) {
                    outputMsg(str_err, str_err_missing_stmts, -1);
                }
                else {
                    if (!isValidStatement(&nodes[NODE.Func.Body])) {
                        outputMsg(str_err, str_err_incomplete_stmts, -1);
                    }
                    else {
                        finishIndent(&nodes[NODE.Func.Body]);
                        NODE_INCR_POS(NODE); // Function is finished
                    }
                }
            }
            else {
                outputMsg(str_err, str_err_wrong_indent, GET_NODE_TYPE_IDX(NODE));
            }
            break;
    }
}

void handleTokenBuiltin(uint8_t *nodeId, parsedToken *token) {
    NodeType *n = &DIRECT_NODE;
    if (SINGLE_ARG_OP(n->Builtin.function)) {
        if (NODE_GET_POS(*n) == 0) {
            n->Builtin.Arg1 = getFreeNode();
            NODE_INCR_POS(*n);
        }
        handleTokenNode(&(n->Builtin.Arg1), token);
    }
    if (n->Builtin.function == BUILTIN_BREAK) {
        outputMsg(str_err, str_err_unexpected_char, GET_NODE_TYPE_IDX(NODE));
    }
}

int16_t tmp;

int16_t evaluateNode(NodeType *node) {
    int16_t retval=0;
    SymbolEntry *var;
    switch((NodeTypeEnum) GET_NODE_TYPE(*node)) {
        case TYPE_VALUE:
            return node->Value.value;
        case TYPE_ID:
            var = getSymbolEntry(GET_ID_HASH(*node));
            if (var == NULL) {
                sevenSegmentPrintHex(0xE8);
                outputMsg(str_err, str_err_undefined_var, -1);
                return -1;
            }
            return var->value;

        case TYPE_ASSIGN:
            var = getOrCreateSymbolEntry(GET_ID_HASH(nodes[node->Assign.Id]));
            var->value = evaluateNode(&nodes[node->Assign.Expr]);
            return var->value;

        case TYPE_EXPR_BRACKET:
        case TYPE_EXPR:
            if (!isValidExpression(node)) {
                outputMsg(str_err, str_err_incomplete_expr, -1);
                return -1;
            }
            return evaluateExpression(node);

        case TYPE_IF:
            if (evaluateNode(&nodes[node->If.Condition])) {
                return evaluateNode(&nodes[node->If.Then]);
            }
            else if (NODE_GET_POS(*node) == 7 || NODE_GET_POS(*node) == 5) {
                return evaluateNode(&nodes[node->If.Else]);
            }
            break;
        case TYPE_WHILE:
            while (evaluateNode(&nodes[node->While.Condition])) {
                retval = evaluateNode(&nodes[node->While.Stmt]);
                if (GOT_RETURN()) {
                    return retval;
                }
                if (GOT_BREAK()) {
                    break;
                }
            }
            CLEAR_BREAK();
            break;

        case TYPE_STMT:
            if (NODE_GET_POS(*node) > 0) {
                retval = evaluateNode(&nodes[node->Stmt.Stmt]);

                if (GOT_RETURN()) {
                    return retval;
                }
                if (STMT_HAS_NEXT(*node)) {
                    retval = evaluateNode(&nodes[node->Stmt.Next]);
                }
            }
            break;

        case TYPE_CALL:
            retval = FUNC_GET_ARGC(*node);
            var = getSymbolEntry(GET_ID_HASH(nodes[node->Call.Id]));
            if (GET_CALL_TYPE(*node) == CALL_FUN) {
                if (var == NULL) {
                    sevenSegmentPrintHex(0xE9);
                    outputMsg(str_err, str_err_undefined_var, -1);
                    return -1;
                }
                if (GET_NODE_TYPE(nodes[(uint8_t) var->value]) != TYPE_FUNC) {
                    sevenSegmentPrintHex(0xE10);
                    outputMsg(str_err, str_err_not_function, -1);
                    return -1;
                }
                if (retval != FUNC_GET_ARGC(nodes[var->value])) {
                    sevenSegmentPrintHex(0xE10);
                    outputMsg(str_err, str_err_wrong_argc, -1);
                    return -1;
                }
            }
            else {
                if (GET_CALL_TYPE(*node) == CALL_GET) {
                    if (retval != 1) {
                        outputMsg(str_err, str_err_wrong_argc, -1);
                        return -1;
                    }
                    else {
                        return (int16_t) *(volatile uint8_t *) evaluateNode(&nodes[*getArg(node, 0)]);
                    }
                }
                if (GET_CALL_TYPE(*node) == CALL_SET) {
                    if (retval != 2) {
                        outputMsg(str_err, str_err_wrong_argc, -1);
                        return -1;
                    }
                    else {
                        (*(volatile uint8_t *)(evaluateNode(&nodes[*getArg(node, 0)]))) = evaluateNode(&nodes[*getArg(node, 1)]);
                        return 1;
                    }
                }
            }

            for (uint8_t i=0; i < FUNC_GET_ARGC(*node); i++) {
                retval = evaluateNode(&nodes[*getArg(node, i)]); // Need to read using variables at current level
                if (GET_CALL_TYPE(*node) == CALL_FUN) {
                    symbolTableDepth++; // Need to insert at next level
                    createSymbolEntry(GET_ID_HASH(nodes[*getArg(&nodes[var->value], i)]), retval);
                    symbolTableDepth--;
                    continue;
                }
                outputChar((char) retval);
            }
            if (GET_CALL_TYPE(*node) != CALL_FUN) { // put function ends here
                return retval;
            }

            symbolTableDepth++;
            retval = evaluateNode(&nodes[nodes[var->value].Func.Body]);
            decreaseSymbolTableDepth(); // Clears variables at current level
            CLEAR_RETURN();

            return retval;

        case TYPE_BUILTIN:
            if (SINGLE_ARG_OP(node->Builtin.function)) {
                retval = evaluateNode(&nodes[node->Builtin.Arg1]);
            }
            if (node->Builtin.function == BUILTIN_RETURN) {
                SET_RETURN();
                return retval;
            }
            if (IS_PRINT_OP(node->Builtin.function)) {
                outputValue(retval, node->Builtin.function != BUILTIN_PRINTN);
                return retval;
            }
            if (node->Builtin.function == BUILTIN_BREAK) {
                SET_BREAK();
            }
            break;

        default:
            outputMsg(str_err, str_err_evaluate, GET_NODE_TYPE_IDX(*node));
    }
    return retval;
}

int8_t getIndentLevel(NodeType *node) {
    while (node != NULL && GET_NODE_TYPE(*node) != TYPE_STMT) {
        node = GET_PARENT(node);
    }
    if (node == NULL) {
        return 0;
    }
    if (NODE_GET_POS(*node) == 0) {
        return -1;
    }
    else {
        return STMT_GET_INDENT(*node);
    }
}

bool isValidStatement(NodeType *node) {
    switch((NodeTypeEnum) GET_NODE_TYPE(*node)) {
        case TYPE_VALUE:
        case TYPE_ID:
            return true;
        case TYPE_CALL:
        case TYPE_EXPR_BRACKET:
        case TYPE_EXPR:
            return isValidExpression(node);
        case TYPE_ASSIGN:
            if (ASSIGN_EMPTY(*node)) {
                return false;
            }
            return isValidExpression(&nodes[node->Assign.Expr]);
        case TYPE_UNUSED:
            return false;

        case TYPE_STMT:
            return isValidStatement(&nodes[node->Stmt.Stmt]) && (!STMT_HAS_NEXT(*node) || isValidStatement(&nodes[node->Stmt.Next]));

        case TYPE_IF:
            if (NODE_GET_POS(*node) > 5) {
                return true;
            }
            if (NODE_GET_POS(*node) == 3) {
                return isValidStatement(&nodes[node->If.Then]);
            }
            if (NODE_GET_POS(*node) == 5) {
                return isValidStatement(&nodes[node->If.Else]);
            }
            return false;

        case TYPE_WHILE:
            if (NODE_GET_POS(*node) == 4) {
                return true;
            }
            if (NODE_GET_POS(*node) == 3) {
                return isValidStatement(&nodes[node->While.Stmt]);
            }
            return false;

        case TYPE_BUILTIN:
            if (SINGLE_ARG_OP(node->Builtin.function)) {
                return isValidExpression(&nodes[node->Builtin.Arg1]);
            }
            if (node->Builtin.function == BUILTIN_BREAK) {
                return true;
            }
            return false;

        default:
            //outputStr("Can't determine if valid statement");
            return false;

    }
    return false;
}

void finishIndent(NodeType *node) {
    switch GET_NODE_TYPE(*node) {
        case TYPE_STMT:
            finishIndent(&nodes[node->Stmt.Stmt]);
            if (STMT_HAS_NEXT(*node)) {
                finishIndent(&nodes[node->Stmt.Next]);
            }
            break;
        case TYPE_IF:
            if (NODE_GET_POS(*node) == 3 && isValidStatement(&nodes[node->If.Then])) {
                finishIndent(&nodes[node->If.Then]);
                NODE_SET_POS(*node, 6);
            }
            if (NODE_GET_POS(*node) == 5 && isValidStatement(&nodes[node->If.Else])) {
                finishIndent(&nodes[node->If.Else]);
                NODE_SET_POS(*node, 7);
            }
            break;
        case TYPE_WHILE:
            if (NODE_GET_POS(*node) == 3 && isValidStatement(&nodes[node->While.Stmt])) {
                finishIndent(&nodes[node->While.Stmt]);
                NODE_SET_POS(*node, 4);
            }
            break;
    }
}