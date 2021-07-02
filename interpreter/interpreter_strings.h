#ifndef INTERPRETER_STRINGS_H
#define INTERPRETER_STRINGS_H
#include "scanner.h"

const char str_err[] PROGMEM = "ERR: ";
const char str_err_undefined_var[] PROGMEM = "Undefined";
const char str_err_consume_eol[] PROGMEM = "EOL type ";
const char str_err_parent[] PROGMEM = "PARENT ";
const char str_err_consume[] PROGMEM = "Consume type ";
const char str_err_evaluate[] PROGMEM = "Eval type ";
const char str_err_free_node[] PROGMEM = "Free node type ";
const char str_err_unexpected_char[] PROGMEM = "Unexpected token";
const char str_err_bracket_mismatch[] PROGMEM = "Bracket mismatch";
const char str_err_incomplete_expr[] PROGMEM = "Incomplete expression";
const char str_err_expect_newline[] PROGMEM = "Expected newline";
const char str_err_expect_colon[] PROGMEM = "Expected :";
const char str_err_missing_stmts[] PROGMEM = "No statements";
const char str_err_incomplete_stmts[] PROGMEM = "Statement incomplete";
const char str_err_wrong_indent[] PROGMEM = "Indent";
const char str_err_expect_id[] PROGMEM = "Expected ID";
const char str_err_expect_bracket[] PROGMEM = "Expected bracket";
const char str_err_not_function[] PROGMEM = "Not a function";
const char str_err_wrong_argc[] PROGMEM = "Wrong arg#";


const char str_active_node[] PROGMEM = "CurNode: ";
const char str_free_nodes[] PROGMEM = "FreeNodes: ";
const char str_free_symbols[] PROGMEM = "FreeSymbols: ";
const char str_next_free[] PROGMEM = "Next node: ";
const char str_node[] PROGMEM = "Node ";
const char str_parent[] PROGMEM = " (From: ";
const char str_pos[] PROGMEM = " (Pos: ";
const char str_indent[] PROGMEM = " (Indent ";

const char str_space_8[] PROGMEM = "        ";
const char str_expr_status[] PROGMEM = "Status = ";
const char str_expr_op[] PROGMEM     = "Opcode = ";
const char str_expr_left[] PROGMEM   = "Left   = ";
const char str_expr_right[] PROGMEM  = "Right  = ";
const char str_expr_done[] PROGMEM   = "Done   = ";
const char str_if_cond[] PROGMEM   = "Cond = ";
const char str_if_then[] PROGMEM   = "Then = ";
const char str_if_else[] PROGMEM   = "Else = ";
const char str_stmt_stmt[] PROGMEM   = "Stmt = ";
const char str_stmt_next[] PROGMEM   = "Next = ";


const char str_node_type_0[] PROGMEM = "UNUSED";
const char str_node_type_1[] PROGMEM = "VALUE";
const char str_node_type_2[] PROGMEM = "IF";
const char str_node_type_3[] PROGMEM = "BUILTIN";
const char str_node_type_4[] PROGMEM = "STMT";
const char str_node_type_5[] PROGMEM = "ASSIGN";
const char str_node_type_6[] PROGMEM = "WHILE";
const char str_node_type_7[] PROGMEM = "FUNC";
const char str_node_type_8[] PROGMEM = "PARAM";
const char str_node_type_9[] PROGMEM = "CALL";
const char str_node_type_10[] PROGMEM = "ID";
const char str_node_type_11[] PROGMEM = "EXPR";
const char str_node_type_12[] PROGMEM = "EXPR_BRACKET";

const char* const str_node_type_table[] PROGMEM = {
    str_node_type_0,
    str_node_type_1,
    str_node_type_2,
    str_node_type_3,
    str_node_type_4,
    str_node_type_5,
    str_node_type_6,
    str_node_type_7,
    str_node_type_8,
    str_node_type_9,
    str_node_type_10,
    str_node_type_11,
    str_node_type_12,
};

#endif