#include "../interpreter.h"
#include "../interpreter_strings.h"

void freeNode(NodeType *node) {
    uint8_t pos = NODE_GET_POS(*node);
    switch ((NodeTypeEnum) GET_NODE_TYPE(*node)) {
        case TYPE_VALUE:
        case TYPE_UNUSED:
        case TYPE_ID:
            break;

        case TYPE_ASSIGN:
            freeNode(&nodes[node->Assign.Id]);
            freeNode(&nodes[node->Assign.Expr]);
            node->Id.value = 0;
            break;

        case TYPE_EXPR_BRACKET:
        case TYPE_EXPR:
            if (pos > 0 && node->Expr.op != OP_NEG && node->Expr.op != OP_NOT) {
                if (pos != 6 || node->Expr.op != OP_NONE) {
                    freeNode(&nodes[node->Expr.left]);
                }
            }
            if (pos >= 4) {
                freeNode(&nodes[node->Expr.right]);
            }
            break;

        case TYPE_IF:
            if (pos > 0) {
                freeNode(&nodes[node->If.Condition]);
            }
            if (pos >= 3) {
                freeNode(&nodes[node->If.Then]);
            }
            if (pos >= 5 && pos != 6) {
                freeNode(&nodes[node->If.Else]);
            }
            break;

        case TYPE_STMT:
            if (pos > 0) {
                freeNode(&nodes[node->Stmt.Stmt]);
            }
            if (STMT_HAS_NEXT(*node)) {
                freeNode(&nodes[node->Stmt.Next]);
            }
            break;
        case TYPE_WHILE:
            if (pos > 0) {
                freeNode(&nodes[node->While.Condition]);
            }
            if (pos > 2) {
                freeNode(&nodes[node->While.Stmt]);
            }
            break;

        case TYPE_FUNC:
            freeNode(&nodes[node->Func.Params]);
            freeNode(&nodes[node->Func.Body]);
            break;

        case TYPE_CALL:
            if (GET_CALL_TYPE(*node) == CALL_FUN) {
                freeNode(&nodes[node->Call.Id]);
            }
            if (FUNC_GET_ARGC(*node) > 0) {
                freeNode(&nodes[node->Call.Args]);
            }
            break;

        case TYPE_PARAM:
            freeNode(&nodes[node->Param.Name1]);
            if (pos > 0) {
                freeNode(&nodes[node->Param.Name2]);
            }
            if (pos > 1) {
                freeNode(&nodes[node->Param.Next]);
            }
            break;

        case TYPE_BUILTIN:
            if (pos > 0) {
                freeNode(&nodes[node->Builtin.Arg1]);
            }
            break;

        default:
            outputMsg(str_err, str_err_free_node, GET_NODE_TYPE_IDX(*node));
    }
    node->Id.value = 0;
}

uint8_t getFreeNode() {
    for (uint8_t i=0; i < nodeCount; i++) {
        if (GET_NODE_TYPE(nodes[i]) == TYPE_UNUSED) {
            return i;
        }
    }
    // Get a new node
    nodes[nodeCount].Id.value = 0;
    return nodeCount++;
}

#define RETURN_PARENT(sub) if (nodes[i].sub == nodeIdx) { if (ptr) { return &nodes[i].sub; } else { return &nodes[i];}};
void *getParentNode(NodeType *node, bool ptr) {
    if (GET_NODE_TYPE(*node) == TYPE_FUNC && NODE_GET_POS(*node) == 7) { // Finished functions don't have a parent
        if (ptr) {
            return &activeNode;
        }
        else {
            return NULL;
        }
    }
    uint8_t nodeIdx = (node-nodes);
    uint8_t pos;
    for (uint8_t i=0; i < nodeCount; i++) {
        pos = NODE_GET_POS(nodes[i]);
        if (GET_NODE_TYPE(nodes[i]) != TYPE_UNUSED) {
            switch((NodeTypeEnum) GET_NODE_TYPE(nodes[i])) {
                case TYPE_UNUSED:
                case TYPE_VALUE:
                case TYPE_ID:
                    break;
                case TYPE_ASSIGN:
                    RETURN_PARENT(Assign.Id);
                    if (!ASSIGN_EMPTY(nodes[i])) {
                        RETURN_PARENT(Assign.Expr);
                    }
                    break;
                case TYPE_EXPR:
                case TYPE_EXPR_BRACKET:
                    if (pos > 0 && nodes[i].Expr.op != OP_NEG && nodes[i].Expr.op != OP_NOT) {
                        if (pos != 6 || nodes[i].Expr.op != OP_NONE) {
                            RETURN_PARENT(Expr.left);
                        }
                    }
                    if (pos >= 4) {
                        RETURN_PARENT(Expr.right);
                    }
                    break;
                case TYPE_IF:
                    if (pos > 0) {
                        RETURN_PARENT(If.Condition);
                    }
                    if (pos > 2) {
                        RETURN_PARENT(If.Then);
                    }
                    if (pos > 4 && pos != 6) {
                        RETURN_PARENT(If.Else);
                    }
                    break;
                case TYPE_STMT:
                    if (pos > 0) {
                        RETURN_PARENT(Stmt.Stmt)
                        if (STMT_HAS_NEXT(nodes[i])) {
                            RETURN_PARENT(Stmt.Next);
                        }
                    }
                    break;

                case TYPE_WHILE:
                    if (pos > 0) {
                        RETURN_PARENT(While.Condition);
                    }
                    if (pos > 2) {
                        RETURN_PARENT(While.Stmt);
                    }
                    break;

                case TYPE_PARAM:
                    RETURN_PARENT(Param.Name1);
                    if (pos > 0) {
                        RETURN_PARENT(Param.Name2);
                    }
                    if (pos > 1) {
                        RETURN_PARENT(Param.Next);
                    }
                    break;

                case TYPE_FUNC:
                    if (FUNC_GET_ARGC(nodes[i]) > 0) {
                        RETURN_PARENT(Func.Params);
                    }
                    if (pos > 5) {
                        RETURN_PARENT(Func.Body);
                    }
                    break;
                case TYPE_CALL:
                    if (GET_CALL_TYPE(nodes[i]) == CALL_FUN) {
                        RETURN_PARENT(Call.Id);
                    }
                    if (FUNC_GET_ARGC(nodes[i]) > 0) {
                        RETURN_PARENT(Call.Args);
                    }

                    break;
                case TYPE_BUILTIN:
                    if (SINGLE_ARG_OP(nodes[i].Builtin.function) && pos > 0) {
                        RETURN_PARENT(Builtin.Arg1);
                    }
            }
        }
    }
    if (activeNode == nodeIdx) {
        if (ptr) {
            return &activeNode;
        }
        else {
            return NULL;
        }
    }
    outputMsg(str_err, str_err_parent, -1);
    return NULL;
}