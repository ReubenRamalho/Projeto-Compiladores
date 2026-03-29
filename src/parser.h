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

/**
 * @brief Inicializa o parser com o código-fonte.
 */
void parser_init(Parser *p, const char *src);

/**
 * @brief Exibe erro de sintaxe e encerra o programa.
 */
void parser_error_at(Parser *p, size_t pos, const char *msg);

/**
 * @brief Consome o token atual e avança para o próximo.
 */
void advance(Parser *p);

/**
 * @brief Verifica se o token atual é o esperado, consumindo-o. Caso contrário, erro.
 */
void expect(Parser *p, TokenKind kind, const char *what);

/**
 * @brief Faz o parsing do programa completo (declarações globais e main).
 */
Program *parse_program(const char *src);

/**
 * @brief Faz o parsing de uma declaração genérica (decide se é 'var' ou 'fun').
 */
Decl *parse_decl(Parser *p);

/**
 * @brief Faz o parsing de uma declaração de variável local ou global ("var id = exp;").
 */
VarDecl *parse_vardecl(Parser *p);

/**
 * @brief Faz o parsing de uma declaração de função ("fun id(args) { ... }").
 */
Decl *parse_fundecl(Parser *p);

/**
 * @brief Faz o parsing de um comando individual.
 */
Cmd *parse_cmd(Parser *p);

/**
 * @brief Faz o parsing de um bloco de comandos.
 */
CmdList parse_cmd_block(Parser *p, TokenKind terminator);

/**
 * @brief Faz o parsing de uma expressão completa (nível relacional).
 */
Expr *parse_exp(Parser *p);

/**
 * @brief Faz o parsing de uma expressão aditiva (+, -).
 */
Expr *parse_exp_a(Parser *p);

/**
 * @brief Faz o parsing de uma expressão multiplicativa (*, /).
 */
Expr *parse_exp_m(Parser *p);

/**
 * @brief Faz o parsing de uma expressão primária (inteiro, parênteses, variável ou chamada de função).
 */
Expr *parse_prim(Parser *p);