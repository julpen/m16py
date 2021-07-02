#include "../interpreter.h"
#include "../interpreter_strings.h"

#define CASE_ID_TO_OP(token, _op) case token: return _op;
int8_t idToOp(keyword id) {
    switch(id) {
        CASE_ID_TO_OP(ID_EXP, OP_EXP);
        CASE_ID_TO_OP(ID_MULT, OP_MULT);
        CASE_ID_TO_OP(ID_DIV, OP_DIV);
        CASE_ID_TO_OP(ID_MOD, OP_MOD);
        CASE_ID_TO_OP(ID_PLUS, OP_PLUS);
        CASE_ID_TO_OP(ID_MINUS, OP_MINUS);
        CASE_ID_TO_OP(ID_SHIFT_LEFT, OP_LSHIFT);
        CASE_ID_TO_OP(ID_SHIFT_RIGHT, OP_RSHIFT);
        CASE_ID_TO_OP(ID_EQU, OP_EQU);
        CASE_ID_TO_OP(ID_NEQ, OP_NEQ);
        CASE_ID_TO_OP(ID_GT, OP_GT);
        CASE_ID_TO_OP(ID_GE, OP_GE);
        CASE_ID_TO_OP(ID_LT, OP_LT);
        CASE_ID_TO_OP(ID_LE, OP_LE);
        CASE_ID_TO_OP(ID_AND, OP_AND);
        CASE_ID_TO_OP(ID_OR, OP_OR);
        default:
            return -1;
    }
}

void handleTokenIdValue(uint8_t *nodeId, parsedToken *token) {
    if (idToOp(token->token) >= 0) {
        transformValIdToExpr(nodeId, idToOp(token->token));
    }
    else if (GET_NODE_TYPE(DIRECT_NODE) == TYPE_VALUE) {
        sevenSegmentPrintHex(0xE5);
    }
    else if (token->token == ID_ASSIGN) {
        NEXT_NODE.Assign.Id = *nodeId;
        nodeId[0] = getFreeNode();
        SET_NODE_TYPE(DIRECT_NODE, TYPE_ASSIGN);
    }
    else if (token->token == ID_OPEN_ROUND) { // Function call
        NEXT_NODE.Call.Id = *nodeId;
        nodeId[0] = getFreeNode();
        SET_NODE_TYPE(DIRECT_NODE, TYPE_CALL);
    }
    else {
        sevenSegmentPrintHex(0xE110);
    }
}

void transformValIdToExpr(uint8_t *nodeId, ExprOperands op) {
    uint8_t newId = getFreeNode();
    COPY_NODE(DIRECT_NODE, nodes[newId]);
    SET_NODE_TYPE(DIRECT_NODE, TYPE_EXPR);
    NODE_SET_POS(DIRECT_NODE, 3);

    DIRECT_NODE.Expr.op = op;
    DIRECT_NODE.Expr.left = newId;
}


void handleTokenExpr(uint8_t *nodeId, parsedToken *token, uint8_t *rootPtr) {
    NodeType *n = &DIRECT_NODE;
    switch(NODE_GET_POS(NODE)) {
        case 0:
            switch(token -> token) {
                case ID_MINUS:
                    NODE.Expr.op = OP_NEG;
                    NODE_SET_POS(NODE, 3);
                    return;
                case ID_NOT:
                    NODE.Expr.op = OP_NOT;
                    NODE_SET_POS(NODE, 3);
                    return;

                case ID_OPEN_ROUND:
                    NODE.Expr.left = getFreeNode();
                    handleTokenNode(&NODE.Expr.left, token);
                    NODE_INCR_POS(NODE);
                    return;
                case ID_NUM:
                case ID_NAME:
                    NODE.Expr.left = getFreeNode();
                    handleTokenNode(&NODE.Expr.left, token);
                    NODE_SET_POS(NODE, 2);
                    return;
                case ID_PLUS:
                    return;
                default:
                    outputMsg(str_err, str_err_unexpected_char, -1);
            }
            return;
        case 1:
            handleTokenExpr(&NODE.Expr.left, token, &NODE.Expr.left);
            if (NODE_GET_POS(nodes[NODE.Expr.left]) == 6) {
                NODE_INCR_POS(NODE);
            }
            return;
        case 2:
            if (idToOp(token->token) >= 0) {
                NODE.Expr.op = idToOp(token->token);
                NODE_INCR_POS(NODE);
            }
            else {
                if (!isValidExpression(&nodes[NODE.Expr.left])) {
                    handleTokenNode(&NODE.Expr.left, token);
                    return;
                }
                else if (token->token == ID_OPEN_ROUND && GET_NODE_TYPE(nodes[NODE.Expr.left]) == TYPE_ID) {
                    handleTokenNode(&NODE.Expr.left, token);
                    return;
                }
                else if (token->token == ID_CLOSE_ROUND && GET_NODE_TYPE(NODE) == TYPE_EXPR_BRACKET) {
                    tmpId = (*nodeId);
                    nodeId[0] = NODE.Expr.left;
                    n = &DIRECT_NODE;
                    nodes[tmpId].Id.value = 0;

                    if (IS_EXPR_NODE(*GET_PARENT(&NODE))) {
                        switch (NODE_GET_POS(*GET_PARENT(&NODE))) {
                            case 1:
                                NODE_INCR_POS(*GET_PARENT(&NODE));
                                break;
                            case 4:
                                if (GET_NODE_TYPE(*GET_PARENT(&NODE)) == TYPE_EXPR_BRACKET) {
                                    NODE_INCR_POS(*GET_PARENT(&NODE));
                                }
                                else {
                                    NODE_SET_POS(*GET_PARENT(&NODE), 6);
                                }
                        }
                    }
                }
                else {
                    outputMsg(str_err, str_err_unexpected_char, -1);
                }
            }
            return;
        case 3:
            switch(token -> token) {
                case ID_NAME:
                case ID_NUM:
                    NODE.Expr.right = getFreeNode();
                    handleTokenNode(&NODE.Expr.right, token);
                    if (GET_NODE_TYPE(NODE) == TYPE_EXPR_BRACKET) {
                        NODE_SET_POS(NODE, 5);
                    }
                    else {
                        NODE_SET_POS(NODE, 6);
                    }
                    return;

                case ID_OPEN_ROUND:
                    NODE.Expr.right = getFreeNode();
                    handleTokenNode(&NODE.Expr.right, token);
                    NODE_INCR_POS(NODE);
                    return;

                case ID_NOT:
                    if (NODE.Expr.op == OP_NOT) { // Just reset a double negation
                        NODE.Expr.op = OP_NONE;
                        NODE_SET_POS(NODE, 0);
                        return;
                    }
                    outputMsg(str_err, str_err_unexpected_char, -1);
                    return;
                case ID_MINUS:
                    if (NODE.Expr.op == OP_NEG) { // Double negation
                        NODE.Expr.op = OP_NONE;
                        NODE_SET_POS(NODE, 0);
                        return;
                    }
                default:
                    outputMsg(str_err, str_err_unexpected_char, -1);
                    return;
            }
            return;
        case 4:
            handleTokenExpr(&NODE.Expr.right, token, &NODE.Expr.right);
            if (NODE_GET_POS(nodes[NODE.Expr.right]) == 6) {
                if (GET_NODE_TYPE(NODE) == TYPE_EXPR_BRACKET) {
                    NODE_INCR_POS(NODE);
                }
                else {
                    NODE_SET_POS(NODE, 6);
                }
            }
            return;
        case 5:
            if (token->token == ID_CLOSE_ROUND) {
                NODE_INCR_POS(NODE);
                return;
            }
        case 6:
            // Traverse to the very last leaf and hand off if there is any unfinished node
            while (IS_EXPR_NODE(nodes[DIRECT_NODE.Expr.right])) {
                nodeId = &NODE.Expr.right;
                if (NODE_GET_POS(DIRECT_NODE) != 6) {
                    handleTokenExpr(nodeId, token, nodeId);
                    return;
                }
            }
            n = &DIRECT_NODE;

            if (!isValidExpression(&nodes[NODE.Expr.right])) {
                handleTokenNode(&NODE.Expr.right, token);
                return;
            }
            if (idToOp(token->token) >= 0) {
                while (nodeId != rootPtr && ((GET_NODE_TYPE(NODE) == TYPE_EXPR_BRACKET && NODE_GET_POS(NODE) == 6) || (idToOp(token->token)>>3) >= (NODE.Expr.op>>3))) {
                    nodeId = GET_PARENT_PTR(GET_PARENT(&DIRECT_NODE));
                }
                n = &DIRECT_NODE;
                tmpId = getFreeNode();

                nodes[tmpId].Expr.op = idToOp(token->token);

                if ((GET_NODE_TYPE(NODE) == TYPE_EXPR_BRACKET && NODE_GET_POS(NODE) == 6) || (idToOp(token->token)>>3) >= (NODE.Expr.op>>3)) {
                    // Evaluate left to right -> New operand becomes new root
                    NEXT_NODE.Expr.left = *nodeId;
                    nodeId[0] = getFreeNode();
                    n = &DIRECT_NODE;
                    if (NODE_GET_POS(nodes[NODE.Expr.left]) == 6) {
                        SET_NODE_TYPE(NODE, TYPE_EXPR);
                    }
                    else {
                        SET_NODE_TYPE(NODE, GET_NODE_TYPE(nodes[NODE.Expr.left]));
                        SET_NODE_TYPE(nodes[NODE.Expr.left], TYPE_EXPR);
                        NODE_SET_POS(nodes[NODE.Expr.left], 6);
                    }
                    NODE_SET_POS(NODE, 3);
                }
                else {
                    // New operand has precedence over old -> Replace right element with new operand
                    NEXT_NODE.Expr.left = NODE.Expr.right;
                    NODE.Expr.right = getFreeNode();
                    SET_NODE_TYPE(NEXT_NODE, TYPE_EXPR);
                    NODE_SET_POS(nodes[NODE.Expr.right], 3);
                }


            } else {
                if (GET_NODE_TYPE(nodes[NODE.Expr.right]) == TYPE_ID && token->token == ID_OPEN_ROUND) {
                    handleTokenIdValue(&NODE.Expr.right, token);
                    return;
                }
                outputMsg(str_err, str_err_unexpected_char, -1);
            }
            return;

        default:
            return;
    }
}

int16_t power(int16_t base, int16_t exp) {
    int16_t retval = 1;
    while (exp > 0) {
        retval = base*retval;
        exp--;
    }
    return retval;
}

#define CASE_EVAL_OP(tok, op) case tok: return left op retval;
int16_t evaluateExpression(NodeType *node) {
    int16_t retval = 0;
    if (node->Expr.op == OP_NEG) {
        return -evaluateNode(&nodes[node->Expr.right]);
    }
    else if (node->Expr.op == OP_NONE) { // This is an unfinished operation
        return evaluateNode(&nodes[node->Expr.left]);
    }
    else if (node->Expr.op == OP_NOT) {
        return !evaluateNode(&nodes[node->Expr.right]);
    }
    else if (node->Expr.op == OP_AND) { // Only evaluate the second expression if the first one is true
        return evaluateNode(&nodes[node->Expr.left]) && evaluateNode(&nodes[node->Expr.right]);
    }
    else if (node->Expr.op == OP_OR) { // Only evaluate the second expression if the first one is false
        return evaluateNode(&nodes[node->Expr.left]) || evaluateNode(&nodes[node->Expr.right]);
    }
    else {
        int16_t left = evaluateNode(&nodes[node->Expr.left]);
        retval = evaluateNode(&nodes[node->Expr.right]);

        switch((ExprOperands) node->Expr.op) {
            CASE_EVAL_OP(OP_PLUS, +);
            CASE_EVAL_OP(OP_MINUS, -);
            CASE_EVAL_OP(OP_MULT, *);
            CASE_EVAL_OP(OP_DIV, /);
            CASE_EVAL_OP(OP_EQU, ==);
            CASE_EVAL_OP(OP_NEQ, !=);
            CASE_EVAL_OP(OP_LSHIFT, <<);
            CASE_EVAL_OP(OP_RSHIFT, >>);
            CASE_EVAL_OP(OP_LT, <);
            CASE_EVAL_OP(OP_LE, <=);
            CASE_EVAL_OP(OP_GT, >);
            CASE_EVAL_OP(OP_GE, >=);
            CASE_EVAL_OP(OP_MOD, %);

            case OP_EXP:
                return power(left, retval);

            case OP_NEG:
            case OP_NONE:
            case OP_OR:
            case OP_AND:
            case OP_NOT:
                break; // These are handled previously, just here to avoid compiler warnings
        }
    }

    // Should be unreachable, but avoid compiler warning
    sevenSegmentPrintHex(0xE202);
    return retval;
}

bool isValidExpression(NodeType *node) {
    if (GET_NODE_TYPE(*node) == TYPE_VALUE || GET_NODE_TYPE(*node) == TYPE_ID) {
        return true;
    }
    if (GET_NODE_TYPE(*node) == TYPE_CALL && NODE_GET_POS(*node) == 7) {
        return true;
    }
    if (GET_NODE_TYPE(*node) != TYPE_EXPR && GET_NODE_TYPE(*node) != TYPE_EXPR_BRACKET) {
        return false;
    }
    if (NODE_GET_POS(*node) != 6) {
        return false;
    }
    if (node->Expr.op != OP_NOT && node->Expr.op != OP_NEG) {
        if (!isValidExpression(&nodes[node->Expr.left])) {
            return false;
        }
    }
    if (!isValidExpression(&nodes[node->Expr.right])) {
        return false;
    }
    return true;
}