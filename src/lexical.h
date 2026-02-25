#pragma once

#include <stdio.h>

/**
 * @brief Enumeração dos tipos de tokens possíveis.
 */
typedef enum {
    TOK_INT,      
    TOK_LPAREN,   
    TOK_RPAREN,   
    TOK_OP,       
    TOK_EOF,      
    TOK_INVALID   
} TokenKind;

/**
 * @brief Estrutura que representa um token.
 */
typedef struct {
    TokenKind kind;   
    long      int_value; 
    char      op;        
    size_t    pos;      
} Token;

/**
 * @brief Estrutura do analisador léxico.
 */
typedef struct {
    const char *src; 
    size_t i;        
} Lexer;

/**
 * @brief Inicializa o analisador léxico.
 * @param lx Ponteiro para o lexer.
 * @param src Código fonte.
 */
void lexer_init(Lexer *lx, const char *src);

/**
 * @brief Pula espaços em branco no código fonte.
 * @param lx Ponteiro para o lexer.
 */
void lexer_skip_ws(Lexer *lx);

/**
 * @brief Retorna o próximo token do código fonte.
 * @param lx Ponteiro para o lexer.
 * @return Próximo token encontrado.
 */
Token lexer_next(Lexer *lx);