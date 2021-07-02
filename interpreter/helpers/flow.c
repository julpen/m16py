
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