#pragma once

#include <stdio.h>

typedef enum {
    TOK_INT,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_OP,
    TOK_EOF,
    TOK_INVALID
} TokenKind;

typedef struct {
    TokenKind kind;
    long      int_value; /* se TOK_INT */
    char      op;        /* se TOK_OP  */
    size_t    pos;       /* índice no texto */
} Token;

typedef struct {
    const char *src;
    size_t i;
} Lexer;

void lexer_init(Lexer *lx, const char *src);

void lexer_skip_ws(Lexer *lx);

Token lexer_next(Lexer *lx);