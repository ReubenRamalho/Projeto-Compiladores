#pragma once

#include <stddef.h>

/**
 * @brief Tipos de expressões da AST.
 */
typedef enum {
    EXPR_INT,
    EXPR_VAR,
    EXPR_BINOP
} ExprKind;

/**
 * @brief Nó de expressão da AST.
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
 * Exemplo:
 *   x = 10 + 2;
 */
typedef struct {
    char *name;
    Expr *value;
} Decl;

/**
 * @brief Nó raiz da AST do programa.
 *
 * Um programa EV contém:
 *   - zero ou mais declarações
 *   - uma expressão final obrigatória
 */
typedef struct {
    Decl   **decls;
    size_t   decl_count;
    size_t   decl_capacity;
    Expr    *result_expr;
} Program;

Expr *expr_int(long v);
Expr *expr_var(const char *name);
Expr *expr_binop(char op, Expr *left, Expr *right);
void expr_free(Expr *e);

Decl *decl_new(const char *name, Expr *value);
void decl_free(Decl *d);

Program *program_new(void);
void program_add_decl(Program *p, Decl *d);
void program_set_result(Program *p, Expr *result_expr);
void program_free(Program *p);