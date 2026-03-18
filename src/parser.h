#pragma once

#include "ast.h"
#include "lexical.h"

/**
 * @brief Estado do parser.
 */
typedef struct {
    const char *src;
    Lexer lx;
    Token cur;
} Parser;

void parser_init(Parser *p, const char *src);
void parser_error_at(Parser *p, size_t pos, const char *msg);
void advance(Parser *p);
void expect(Parser *p, TokenKind kind, const char *what);

Program *parse_program(const char *src);
Decl *parse_decl(Parser *p);
Cmd *parse_cmd(Parser *p);
CmdList parse_cmd_block(Parser *p, TokenKind terminator);
Expr *parse_exp(Parser *p);
Expr *parse_exp_a(Parser *p);
Expr *parse_exp_m(Parser *p);
Expr *parse_prim(Parser *p);
