#pragma once

#include <stddef.h>

/**
 * @brief Tipos de tokens da linguagem Fun.
 * 
 */
typedef enum {
    TOK_INT,
    TOK_IDENT,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_RETURN,
    TOK_FUN,     // Função
    TOK_VAR,     // Variável
    TOK_MAIN,    // Main
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_COMMA,   // Ponto e vírgula
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

/**
 * @brief Inicializa o estado do analisador léxico.
 */
void lexer_init(Lexer *lx, const char *src);

/**
 * @brief Pula espaços em branco e quebras de linha.
 */
void lexer_skip_ws(Lexer *lx);

/**
 * @brief Retorna o próximo token encontrado no código-fonte.
 * 
 */
Token lexer_next(Lexer *lx);

/**
 * @brief Libera a memória alocada para o lexema do token.
 */
void token_free(Token *t);