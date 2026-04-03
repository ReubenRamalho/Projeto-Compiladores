#pragma once

/*
 * Estruturas da AST (Árvore Sintática Abstrata) usadas pelo compilador
 * para representar expressões, comandos, declarações e o programa final.
 */

#include <stddef.h>

/**
 * @brief Operadores binários suportados pela linguagem.
 */
typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_LT,
    OP_GT,
    OP_EQ,
    OP_LE,
    OP_GE,
    OP_NE,
    OP_AND,
    OP_OR
} BinOpKind;

/**
 * @brief Operadores unários suportados pela linguagem.
 */
typedef enum {
    UOP_NOT
} UnOpKind;

/**
 * @brief Tipos de expressão da AST.
 */
typedef enum {
    EXPR_INT,
    EXPR_BOOL,
    EXPR_VAR,
    EXPR_BINOP,
    EXPR_UNOP,
    EXPR_CALL,
    EXPR_ARRAY_ACCESS
} ExprKind;

struct Expr;
typedef struct Expr Expr;

/**
 * @brief Lista dinâmica de expressões.
 */
typedef struct {
    Expr **items;
    size_t count;
    size_t capacity;
} ExprList;

/**
 * @brief Nó de expressão da AST.
 */
struct Expr {
    ExprKind kind;
    union {
        long int_value;
        int bool_value;
        char *var_name;
        struct {
            BinOpKind op;
            Expr *left;
            Expr *right;
        } binop;
        struct {
            UnOpKind op;
            Expr *operand;
        } unop;
        struct {
            char *fun_name;
            ExprList args;
        } call;
        struct {                  
            char *array_name;
            Expr *index;
        } array_access;
    } as;
};

/**
 * @brief Declaração de variável.
 */
typedef struct VarDecl {
    char *name;
    Expr *value;
    int is_array;     
    size_t array_size;
} VarDecl;

/**
 * @brief Lista dinâmica de declarações de variáveis.
 */
typedef struct {
    VarDecl **items;
    size_t count;
    size_t capacity;
} VarDeclList;

/**
 * @brief Lista dinâmica de strings.
 */
typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} StringList;

/**
 * @brief Tipos de comandos da linguagem.
 */
typedef enum {
    CMD_ASSIGN,
    CMD_IF,
    CMD_WHILE,
    CMD_ARRAY_ASSIGN
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
        struct {            
            char *name;
            Expr *index;
            Expr *value;
        } array_assign;
    } as;
};

/**
 * @brief Tipos de declarações globais.
 */
typedef enum {
    DECL_VAR,
    DECL_FUN
} DeclKind;

/**
 * @brief Nó de declaração global.
 */
typedef struct Decl {
    DeclKind kind;
    union {
        VarDecl var_decl;
        struct {
            char *name;
            StringList params;
            VarDeclList locals;
            CmdList body;
            Expr *result_expr;
        } fun_decl;
    } as;
} Decl;

/**
 * @brief Programa completo.
 */
typedef struct {
    Decl **decls;
    size_t decl_count;
    size_t decl_capacity;
    CmdList main_body;
    Expr *main_result;
} Program;

void expr_list_init(ExprList *list);
void expr_list_add(ExprList *list, Expr *expr);
void expr_list_free(ExprList *list);

Expr *expr_int(long v);
Expr *expr_bool(int v);
Expr *expr_var(const char *name);
Expr *expr_binop(BinOpKind op, Expr *left, Expr *right);
Expr *expr_unop(UnOpKind op, Expr *operand);
Expr *expr_call(const char *fun_name, const ExprList *args);

Expr *expr_array_access(const char *name, Expr *index); // NOVO

/**
 * @brief Libera a memória de uma expressão
 */
void expr_free(Expr *e);

void string_list_init(StringList *list);
void string_list_add(StringList *list, char *str);
void string_list_free(StringList *list);

VarDecl *var_decl_new(const char *name, Expr *value);

VarDecl *var_decl_array_new(const char *name, size_t size);
void var_decl_free(VarDecl *vd);
void var_decl_list_init(VarDeclList *list);
void var_decl_list_add(VarDeclList *list, VarDecl *vd);
void var_decl_list_free(VarDeclList *list);

void cmd_list_init(CmdList *list);
void cmd_list_add(CmdList *list, Cmd *cmd);
void cmd_list_free(CmdList *list);
Cmd *cmd_assign(const char *name, Expr *value);

Cmd *cmd_array_assign(const char *name, Expr *index, Expr *value);
Cmd *cmd_if(Expr *condition, const CmdList *then_branch, const CmdList *else_branch);
Cmd *cmd_while(Expr *condition, const CmdList *body);
void cmd_free(Cmd *cmd);

Decl *decl_var_new(const char *name, Expr *value);
Decl *decl_fun_new(const char *name, const StringList *params, const VarDeclList *locals, const CmdList *body, Expr *result_expr);
void decl_free(Decl *d);

Program *program_new(void);
void program_add_decl(Program *p, Decl *d);
void program_set_main(Program *p, const CmdList *main_body, Expr *main_result);
void program_free(Program *p);
