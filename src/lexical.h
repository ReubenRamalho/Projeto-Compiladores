#pragma once

#include <stddef.h>

/**
 * @brief Tipos de tokens da linguagem Fun estendida.
 */
typedef enum {
    TOK_INT,
    TOK_IDENT,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_RETURN,
    TOK_FUN,
    TOK_VAR,
    TOK_MAIN,
    TOK_TRUE,
    TOK_FALSE,
    TOK_AND,
    TOK_OR,
    TOK_NOT,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_LBRACKET, // '['
    TOK_RBRACKET, // ']'
    TOK_COMMA,
    TOK_OP_LE,
    TOK_OP_GE,
    TOK_OP_NE,
    TOK_OP_ADD,
    TOK_OP_SUB,
    TOK_OP_MUL,
    TOK_OP_DIV,
    TOK_OP_LT,
    TOK_OP_GT,
    TOK_OP_EQ,
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
    long int_value;
    char *lexeme;
    size_t pos;
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
