#pragma once

#include <stddef.h>

/**
 * @brief Tipos de tokens da linguagem EV.
 *
 * Além dos tokens aritméticos básicos, a linguagem EV adiciona três novos tipos: 
 * ponto-e-vírgula, sinal de igual (usado para atribuição) e identificadores[cite: 55].
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
 * * Armazena o tipo do token e os dados associados a ele, como o valor numérico
 * (se for um inteiro), o operador, ou o lexema (texto original do identificador).
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
 * * Mantém o ponteiro para o código-fonte original e a posição atual (índice)
 * durante a varredura dos caracteres.
 */
typedef struct {
    const char *src;
    size_t i;
} Lexer;

/**
 * @brief Inicializa o analisador léxico com o código-fonte fornecido.
 * @param lx Ponteiro para o estado do lexer.
 * @param src String contendo o código-fonte.
 */
void lexer_init(Lexer *lx, const char *src);

/**
 * @brief Avança o índice do lexer, ignorando espaços em branco, tabulações e quebras de linha.
 * @param lx Ponteiro para o estado do lexer.
 */
void lexer_skip_ws(Lexer *lx);

/**
 * @brief Lê o próximo token do código-fonte.
 * @param lx Ponteiro para o estado do lexer.
 * @return O token lido. Retorna um token com kind TOK_EOF ao final da string.
 */
Token lexer_next(Lexer *lx);

/**
 * @brief Libera a memória alocada dinamicamente para os atributos de um token (como o lexema).
 * @param t Ponteiro para o token.
 */
void token_free(Token *t);