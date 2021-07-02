// Set a new argument/parameter from token
// Can be used both for function definitions or function calls
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

// Get the node that references the argument at idx of the function definition/call
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

// Get the node that references the last currently defined argument
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