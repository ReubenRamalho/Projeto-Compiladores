#pragma once

#include "ast.h"
#include "lexical.h"

/**
 * @brief Estado do analisador sintático (parser).
 * * Mantém o código-fonte original, a instância do analisador léxico 
 * e o token atual que está sendo processado na gramática.
 */
typedef struct {
    const char *src;
    Lexer lx;
    Token cur;
} Parser;

/**
 * @brief Inicializa o parser com o código-fonte.
 * @param p Ponteiro para o estado do parser.
 * @param src String contendo o código-fonte do programa.
 */
void parser_init(Parser *p, const char *src);

/**
 * @brief Reporta um erro de sintaxe e encerra o programa.
 * @param p Ponteiro para o estado do parser.
 * @param pos Posição (índice) onde o erro ocorreu.
 * @param msg Mensagem descritiva do erro.
 */
void parser_error_at(Parser *p, size_t pos, const char *msg);

/**
 * @brief Consome o token atual e avança para o próximo token no lexer.
 * @param p Ponteiro para o estado do parser.
 */
void advance(Parser *p);

/**
 * @brief Verifica se o token atual é do tipo esperado e, em caso afirmativo, não faz nada.
 * Se for de um tipo diferente, dispara um erro sintático.
 * @param p Ponteiro para o estado do parser.
 * @param k Tipo de token esperado.
 * @param what Descrição em texto do que era esperado (para a mensagem de erro).
 */
void expect(Parser *p, TokenKind k, const char *what);

/**
 * @brief Inicia a análise sintática do programa inteiro.
 * * Um programa é uma sequência de zero ou mais declarações, terminando por uma expressão final obrigatória[cite: 63].
 * @param src O código-fonte do programa.
 * @return Ponteiro para a AST raiz (Program) ou encerra em caso de erro.
 */
Program *parse_program(const char *src);

/**
 * @brief Analisa uma declaração de variável.
 * * A função reconhece um nome de variável, depois um sinal de igual, depois uma expressão, e por fim o ponto-e-virgula[cite: 83].
 * @param p Ponteiro para o estado do parser.
 * @return Ponteiro para o nó de declaração (Decl).
 */
Decl *parse_decl(Parser *p);

/**
 * @brief Analisa expressões de adição e subtração (maior escopo).
 * @param p Ponteiro para o estado do parser.
 * @return Ponteiro para a expressão analisada (Expr).
 */
Expr *parse_exp_a(Parser *p);

/**
 * @brief Analisa expressões de multiplicação e divisão (maior precedência).
 * @param p Ponteiro para o estado do parser.
 * @return Ponteiro para a expressão analisada (Expr).
 */
Expr *parse_exp_m(Parser *p);

/**
 * @brief Analisa expressões primárias.
 * * Um novo tipo de expressão primária é uma variável[cite: 61].
 * Pode ser um número, um identificador (variável) ou uma expressão entre parênteses.
 * @param p Ponteiro para o estado do parser.
 * @return Ponteiro para a expressão analisada (Expr).
 */
Expr *parse_prim(Parser *p);