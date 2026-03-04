#pragma once

#include "ast.h"
#include "lexical.h"


/**
 * @brief Estrutura que representa o estado do parser.
 */
typedef struct {
    const char *src; ///< Fonte do código a ser analisado.
    Lexer lx;        ///< Estrutura do analisador léxico.
    Token cur;       ///< Token atual.
} Parser;

/**
 * @brief Inicializa o parser com o código fonte.
 * @param p Ponteiro para o parser.
 * @param src Código fonte a ser analisado.
 */
void parser_init(Parser *p, const char *src);

/**
 * @brief Exibe um erro de parsing em uma posição específica.
 * @param p Ponteiro para o parser.
 * @param pos Posição no código fonte.
 * @param msg Mensagem de erro.
 */
void parser_error_at(Parser *p, size_t pos, const char *msg);

/**
 * @brief Garante que o próximo token seja do tipo esperado.
 * @param p Ponteiro para o parser.
 * @param k Tipo de token esperado.
 * @param what Descrição do que era esperado.
 */
void expect(Parser *p, TokenKind k, const char *what);

/**
 * @brief Avança para o próximo token.
 * @param p Ponteiro para o parser.
 */
void advance(Parser *p);

/**
 * @brief Analisa uma expressão aditiva.
 * @param p Ponteiro para o parser.
 * @return Ponteiro para o nó da AST correspondente.
 */
Node *parse_exp_a(Parser *p);

/**
 * @brief Analisa uma expressão multiplicativa.
 * @param p Ponteiro para o parser.
 * @return Ponteiro para o nó da AST correspondente.
 */
Node *parse_exp_m(Parser *p);

/**
 * @brief Analisa um valor primitivo (número, parênteses, etc).
 * @param p Ponteiro para o parser.
 * @return Ponteiro para o nó da AST correspondente.
 */
Node *parse_prim(Parser *p);

/**
 * @brief Analisa todo o programa a partir do código fonte.
 * @param src Código fonte.
 * @return Ponteiro para o nó raiz da AST.
 */
Node *parse_program(const char *src);