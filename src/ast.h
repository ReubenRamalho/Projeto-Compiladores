#pragma once

#include <stddef.h>

/**
 * @brief Operadores binários suportados pela linguagem Cmd.
 */
typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_LT,
    OP_GT,
    OP_EQ
} BinOpKind;

/**
 * @brief Tipos de expressão da AST.
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
            BinOpKind op;
            struct Expr *left;
            struct Expr *right;
        } binop;
    } as;
} Expr;

/**
 * @brief Declaração de variável feita antes do corpo do programa.
 */
typedef struct {
    char *name;
    Expr *value;
} Decl;

/**
 * @brief Tipos de comandos da linguagem Cmd.
 */
typedef enum {
    CMD_ASSIGN,
    CMD_IF,
    CMD_WHILE
} CmdKind;

struct Cmd;
typedef struct Cmd Cmd;

/**
 * @brief Lista dinâmica de comandos.
 */
typedef struct {
    Cmd **items;
    size_t count;
    size_t capacity;
} CmdList;

/**
 * @brief Nó de comando da AST.
 */
struct Cmd {
    CmdKind kind;
    union {
        struct {
            char *name;
            Expr *value;
        } assign;
        struct {
            Expr *condition;
            CmdList then_branch;
            CmdList else_branch;
        } if_cmd;
        struct {
            Expr *condition;
            CmdList body;
        } while_cmd;
    } as;
};

/**
 * @brief Programa completo da linguagem Cmd.
 *
 * Um programa possui:
 * - zero ou mais declarações iniciais;
 * - um bloco com zero ou mais comandos;
 * - uma expressão final introduzida por return.
 */
typedef struct {
    Decl **decls;
    size_t decl_count;
    size_t decl_capacity;
    CmdList body;
    Expr *result_expr;
} Program;

Expr *expr_int(long v);
Expr *expr_var(const char *name);
Expr *expr_binop(BinOpKind op, Expr *left, Expr *right);
void expr_free(Expr *e);

Decl *decl_new(const char *name, Expr *value);
void decl_free(Decl *d);

void cmd_list_init(CmdList *list);
void cmd_list_add(CmdList *list, Cmd *cmd);
void cmd_list_free(CmdList *list);

Cmd *cmd_assign(const char *name, Expr *value);
Cmd *cmd_if(Expr *condition, const CmdList *then_branch, const CmdList *else_branch);
Cmd *cmd_while(Expr *condition, const CmdList *body);
void cmd_free(Cmd *cmd);

Program *program_new(void);
void program_add_decl(Program *p, Decl *d);
void program_set_body(Program *p, const CmdList *body);
void program_set_result(Program *p, Expr *result_expr);
void program_free(Program *p);
