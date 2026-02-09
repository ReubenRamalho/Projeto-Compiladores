#pragma once

#include "ast.h"
#include "lexical.h"

typedef struct {
    const char *src;
    Lexer lx;
    Token cur;
} Parser;

void parser_init(Parser *p, const char *src);

void parser_error_at(Parser *p, size_t pos, const char *msg);

void expect(Parser *p, TokenKind k, const char *what);

void advance(Parser *p);

Node *parse_expr(Parser *p);

Node *parse_program(const char *src);