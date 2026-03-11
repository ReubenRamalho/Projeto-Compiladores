#pragma once

#include <stddef.h>

/**
 * @brief Tipos de tokens da linguagem EV.
 */
typedef enum {
    TOK_INT,
    TOK_IDENT,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_OP,
    TOK_EQUAL,
    TOK_SEMI,
    TOK_EOF,
    TOK_INVALID
} TokenKind;

/**
 * @brief Token produzido pelo analisador léxico.
 */
typedef struct {
    TokenKind kind;
    long      int_value;
    char      op;
    char     *lexeme;
    size_t    pos;
} Token;

/**
 * @brief Estado do analisador léxico.
 */
typedef struct {
    const char *src;
    size_t i;
} Lexer;

void lexer_init(Lexer *lx, const char *src);
void lexer_skip_ws(Lexer *lx);
Token lexer_next(Lexer *lx);
void token_free(Token *t);