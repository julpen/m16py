#include <interpreter.h>
#include "../interpreter_strings.h"

void (*outputChar)(char);

void outputValue(int16_t val, bool newline) {
    if (val == 0) {
        outputChar('0');
    }
    else {
        if (val < 0) {
            outputChar('-');
            val = -val;
        }
        uint16_t divider = 10000;
        while ((val/divider) == 0) {
            divider = divider/10;
        }
        while (divider > 0) {
            outputChar('0' + ((((uint16_t) val)/divider) % 10));
            divider = divider/10;
        }
    }
    if (newline) {
        outputChar('\n');
    }
};

void outputPgmspace(const char *s) {
    char c;
    for (uint8_t i = 0; i < strlen_P(s); i++) {
        c = pgm_read_byte_near(s+i);
        outputChar(c);
    }
}

void outputMsg(const char *t1, const char *t2, int8_t nodeType) {
    outputPgmspace(t1);
    if (t2 != NULL) {
        outputPgmspace(t2);
    }
    if (nodeType != -1) {
        outputChar(' ');
        outputPgmspace((char *)pgm_read_word(&(str_node_type_table[nodeType])));
    }
    outputChar('\n');
}

void printExpressionNode(NodeType *node) {
    outputPgmspace(str_space_8);
    outputPgmspace(str_expr_op);
    switch(node->Expr.op) {
        case OP_PLUS:
            outputChar('+');
            break;
        case OP_MINUS:
            outputChar('-');
            break;
        case OP_MULT:
            outputChar('*');
            break;
        case OP_DIV:
            outputChar('/');
            break;
        case OP_NONE:
            outputChar('X');
            break;
        case OP_NEG:
            outputChar('N');
            break;
        default:
            outputValue(node->Expr.op, false);
    }
    outputChar('\n');
    outputPgmspace(str_space_8);
    outputPgmspace(str_expr_left);
    outputValue(node->Expr.left, true);
    outputPgmspace(str_space_8);
    outputPgmspace(str_expr_right);
    outputValue(node->Expr.right, true);
    outputPgmspace(str_space_8);
    outputPgmspace(str_expr_status);
    outputValue(node->Expr.op, true);
}

void printIfNode(NodeType *node) {
    if (NODE_GET_POS(*node) > 0) {
        outputPgmspace(str_space_8);
        outputPgmspace(str_if_cond);
        outputValue(node->If.Condition, true);
    }
    if (NODE_GET_POS(*node) > 2) {
        outputPgmspace(str_space_8);
        outputPgmspace(str_if_then);
        outputValue(node->If.Then, true);
    }
    if (NODE_GET_POS(*node) > 4 && NODE_GET_POS(*node) != 6) {
        outputPgmspace(str_space_8);
        outputPgmspace(str_if_else);
        outputValue(node->If.Else, true);
    }
}
void printWhileNode(NodeType *node) {
    if (NODE_GET_POS(*node) > 0) {
        outputPgmspace(str_space_8);
        outputPgmspace(str_if_cond);
        outputValue(node->While.Condition, true);
    }
    if (NODE_GET_POS(*node) > 3) {
        outputPgmspace(str_space_8);
        outputPgmspace(str_stmt_stmt);
        outputValue(node->While.Stmt, true);
    }
}

void printStmtNode(NodeType *node) {
    if (NODE_GET_POS(*node) > 0) {
        outputPgmspace(str_space_8);
        outputPgmspace(str_stmt_stmt);
        outputValue(node->Stmt.Stmt, true);
    }
    if (STMT_HAS_NEXT(*node)) {
        outputPgmspace(str_space_8);
        outputPgmspace(str_stmt_next);
        outputValue(node->Stmt.Next, true);
    }
}

void printDebugInfo() {
    uint8_t freeNodes = 0;
    for (uint8_t i=0; i < nodeCount; i++) {
        if (GET_NODE_TYPE(nodes[i]) == TYPE_UNUSED) {
            freeNodes++;
        }
    }
    outputPgmspace(str_free_nodes);
    outputValue(freeNodes, false);
    outputChar('/');
    outputValue(nodeCount, true);

    freeNodes = 0;
    for (uint8_t i=0; i < MAX_SYMBOLS; i++) {
        if (!(symbolTable[i].name & 0x80000000)) {
            freeNodes++;
        }
    }
    outputPgmspace(str_free_symbols);
    outputValue(freeNodes, false);
    outputChar('/');
    outputValue(MAX_SYMBOLS, true);

    outputPgmspace(str_next_free);
    outputValue(getFreeNode(), true);

    for (uint8_t i=0; i < nodeCount; i++) {
        if (GET_NODE_TYPE(nodes[i]) != TYPE_UNUSED) {
            int type = GET_NODE_TYPE(nodes[i]);
            outputValue(getIndentLevel(&nodes[i]), false);
            outputChar(':');
            outputChar(' ');

            if (i == activeNode) {
                outputChar('*');
            }
            else {
                outputChar(' ');
            }
            outputPgmspace(str_node);
            outputValue(i, false);
            outputChar(':');
            outputChar(' ');
            outputPgmspace((char *)pgm_read_word(&(str_node_type_table[GET_NODE_TYPE_IDX(nodes[i])])));


            if (type == TYPE_VALUE) {
                outputChar('=');
                outputValue(nodes[i].Value.value, false);
            }

            if (type == TYPE_ASSIGN) {
                outputChar(' ');
                outputChar('(');
                outputValue(nodes[i].Assign.Id, false);
                outputChar('=');
                outputValue(nodes[i].Assign.Expr, false);
                outputChar(')');
            }

            outputPgmspace(str_pos);
            outputValue(NODE_GET_POS(nodes[i]), false);
            outputChar(')');


            if (GET_PARENT(&nodes[i]) != NULL) {
                outputPgmspace(str_parent);
                outputValue(GET_PARENT(&nodes[i]) - nodes, false);
                outputChar(')');
            }

            outputChar('\n');
            if (IS_EXPR_NODE(nodes[i])) {
                printExpressionNode(&nodes[i]);
            }
            if (GET_NODE_TYPE(nodes[i]) == TYPE_IF) {
                printIfNode(&nodes[i]);
            }
            if (GET_NODE_TYPE(nodes[i]) == TYPE_STMT) {
                printStmtNode(&nodes[i]);
            }
            if (GET_NODE_TYPE(nodes[i]) == TYPE_WHILE) {
                printWhileNode(&nodes[i]);
            }
        }
    }
}