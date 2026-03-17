#pragma once

#include <stddef.h>

/**
 * @brief Tipos de expressões da AST.
 *
 * A linguagem EV adiciona a possibilidade de declarar e usar variáveis[cite: 16].
 * Dessa forma, temos tipos para inteiros, variáveis e operações binárias.
 */
typedef enum {
    EXPR_INT,
    EXPR_VAR,
    EXPR_BINOP
} ExprKind;

/**
 * @brief Nó de expressão da árvore de sintaxe abstrata (AST).
 *
 * Um novo tipo de expressão primária na linguagem EV é uma variável[cite: 61].
 * O nó armazena o tipo da expressão (kind) e uma união com os dados 
 * específicos correspondentes (valor inteiro, nome da variável ou operandos binários).
 */
typedef struct Expr {
    ExprKind kind;
    union {
        long int_value;
        char *var_name;
        struct {
            char op;
            struct Expr *left;
            struct Expr *right;
        } binop;
    } as;
} Expr;

/**
 * @brief Representa uma declaração de variável.
 *
 * Cada declaração começa com um identificador (usado como nome de variável) seguido por um sinal de igual e uma expressão[cite: 40].
 * O nó da árvore sintática para uma declaração deve guardar o nome da variável e a expressão[cite: 84].
 */
typedef struct {
    char *name;
    Expr *value;
} Decl;

/**
 * @brief Nó raiz da árvore de sintaxe abstrata do programa[cite: 82].
 *
 * Um programa é uma sequência de zero ou mais declarações, terminando por uma expressão final obrigatória[cite: 63].
 * Este nó encapsula as declarações em um array dinâmico e guarda um ponteiro para a expressão final[cite: 82].
 */
typedef struct {
    Decl   **decls;
    size_t   decl_count;
    size_t   decl_capacity;
    Expr    *result_expr;
} Program;

/**
 * @brief Cria um nó de expressão do tipo inteiro.
 * @param v O valor numérico do inteiro.
 * @return Ponteiro para o nó de expressão criado.
 */
Expr *expr_int(long v);

/**
 * @brief Cria um nó de expressão do tipo variável.
 * @param name O identificador (nome) da variável.
 * @return Ponteiro para o nó de expressão criado.
 */
Expr *expr_var(const char *name);

/**
 * @brief Cria um nó de expressão do tipo operação binária.
 * @param op O caractere representando a operação ('+', '-', '*', '/').
 * @param left Ponteiro para a expressão do lado esquerdo.
 * @param right Ponteiro para a expressão do lado direito.
 * @return Ponteiro para o nó de expressão criado.
 */
Expr *expr_binop(char op, Expr *left, Expr *right);

/**
 * @brief Libera a memória alocada por um nó de expressão e seus sub-nós.
 * @param e Ponteiro para a expressão a ser liberada.
 */
void expr_free(Expr *e);

/**
 * @brief Cria um novo nó de declaração de variável.
 * @param name Nome da variável sendo declarada.
 * @param value Expressão atribuída à variável.
 * @return Ponteiro para o nó de declaração criado.
 */
Decl *decl_new(const char *name, Expr *value);

/**
 * @brief Libera a memória alocada por um nó de declaração.
 * @param d Ponteiro para a declaração a ser liberada.
 */
void decl_free(Decl *d);

/**
 * @brief Inicializa o nó raiz do programa.
 * @return Ponteiro para a estrutura Program recém-alocada.
 */
Program *program_new(void);

/**
 * @brief Adiciona uma nova declaração à lista de declarações do programa.
 * @param p Ponteiro para o programa.
 * @param d Ponteiro para a declaração a ser adicionada.
 */
void program_add_decl(Program *p, Decl *d);

/**
 * @brief Define a expressão de resultado final do programa.
 * * O resultado do programa é o resultado da expressão final[cite: 50].
 * @param p Ponteiro para o programa.
 * @param result_expr Ponteiro para a expressão final.
 */
void program_set_result(Program *p, Expr *result_expr);

/**
 * @brief Libera toda a memória associada à AST do programa (declarações e expressão final).
 * @param p Ponteiro para o programa a ser liberado.
 */
void program_free(Program *p);