#pragma once

#include <stddef.h>

/**
 * @brief Operadores binários suportados.
 */
typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_LT,
    OP_GT,
    OP_EQ,
    OP_LE,   // Novo: Menor ou igual
    OP_GE,   // Novo: Maior ou igual
    OP_NE    // Novo: Diferente
} BinOpKind;

/**
 * @brief Tipos de expressão da AST. Adicionado EXPR_CALL para chamadas de função.
 */
typedef enum {
    EXPR_INT,
    EXPR_VAR,
    EXPR_BINOP,
    EXPR_CALL,
    EXPR_ARRAY_ACCESS
} ExprKind;

struct Expr;
typedef struct Expr Expr;

/**
 * @brief Lista dinâmica de expressões (usada para os argumentos reais de uma chamada).
 */
typedef struct {
    Expr **items;
    size_t count;
    size_t capacity;
} ExprList;

/**
 * @brief Nó de expressão da AST. Agora contém a união 'call' para chamadas de função.
 */
struct Expr {
    ExprKind kind;
    union {
        long int_value;
        char *var_name;
        struct {
            BinOpKind op;
            Expr *left;
            Expr *right;
        } binop;
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
 * @brief Declaração de variável (antigo Decl, renomeado para diferenciar de funções).
 */
typedef struct VarDecl {
    char *name;
    Expr *value;
    int is_array;     
    size_t array_size;
} VarDecl;

/**
 * @brief Lista dinâmica de declarações de variáveis (usada para variáveis locais de uma função).
 */
typedef struct {
    VarDecl **items;
    size_t count;
    size_t capacity;
} VarDeclList;

/**
 * @brief Lista dinâmica de strings (usada para os nomes dos parâmetros formais de uma função).
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
 * @brief Tipos de declarações que podem compor um programa no escopo global.
 */
typedef enum {
    DECL_VAR,
    DECL_FUN
} DeclKind;

/**
 * @brief Nó genérico para declarações do topo do programa (variável ou função).
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
 * @brief Programa completo. Contém lista mista de declarações e o bloco principal (main).
 */
typedef struct {
    Decl **decls;
    size_t decl_count;
    size_t decl_capacity;
    CmdList main_body;
    Expr *main_result;
} Program;


/**
 * @brief Inicializa uma lista de expressões.
 */
void expr_list_init(ExprList *list);

/**
 * @brief Adiciona uma expressão à lista.
 */
void expr_list_add(ExprList *list, Expr *expr);

/**
 * @brief Libera a memória de uma lista de expressões e de seus itens.
 */
void expr_list_free(ExprList *list);

/**
 * @brief Cria um nó de expressão inteira literal.
 */
Expr *expr_int(long v);

/**
 * @brief Cria um nó de expressão de variável.
 */
Expr *expr_var(const char *name);

/**
 * @brief Cria um nó de expressão de operação binária.
 */
Expr *expr_binop(BinOpKind op, Expr *left, Expr *right);

/**
 * @brief Cria um nó de expressão de chamada de função.
 */
Expr *expr_call(const char *fun_name, const ExprList *args);

Expr *expr_array_access(const char *name, Expr *index); // NOVO

/**
 * @brief Libera a memória de uma expressão
 */
void expr_free(Expr *e);

/**
 * @brief Inicializa uma lista de strings.
 */
void string_list_init(StringList *list);

/**
 * @brief Adiciona uma string à lista.
 */
void string_list_add(StringList *list, char *str);

/**
 * @brief Libera a memória de uma lista de strings e de seus itens.
 */
void string_list_free(StringList *list);

/**
 * @brief Cria uma nova declaração de variável (antigo decl_new).
 */
VarDecl *var_decl_new(const char *name, Expr *value);

VarDecl *var_decl_array_new(const char *name, size_t size);

/**
 * @brief Libera a memória de uma declaração de variável (antigo decl_free).
 */
void var_decl_free(VarDecl *vd);

/**
 * @brief Inicializa uma lista de declarações de variáveis.
 */
void var_decl_list_init(VarDeclList *list);

/**
 * @brief Adiciona uma declaração de variável à lista.
 */
void var_decl_list_add(VarDeclList *list, VarDecl *vd);

/**
 * @brief Libera a memória de uma lista de declarações de variáveis e seus itens.
 */
void var_decl_list_free(VarDeclList *list);

/**
 * @brief Inicializa uma lista de comandos.
 */
void cmd_list_init(CmdList *list);

/**
 * @brief Adiciona um comando à lista.
 */
void cmd_list_add(CmdList *list, Cmd *cmd);

/**
 * @brief Libera a memória de uma lista de comandos e de seus itens.
 */
void cmd_list_free(CmdList *list);

/**
 * @brief Cria um comando de atribuição.
 */
Cmd *cmd_assign(const char *name, Expr *value);

Cmd *cmd_array_assign(const char *name, Expr *index, Expr *value);

/**
 * @brief Cria um comando condicional if/else.
 */
Cmd *cmd_if(Expr *condition, const CmdList *then_branch, const CmdList *else_branch);

/**
 * @brief Cria um comando de repetição while.
 */
Cmd *cmd_while(Expr *condition, const CmdList *body);

/**
 * @brief Libera a memória de um comando.
 */
void cmd_free(Cmd *cmd);

/**
 * @brief Cria um nó genérico de declaração contendo uma variável.
 */
Decl *decl_var_new(const char *name, Expr *value);

/**
 * @brief Cria um nó genérico de declaração contendo uma função.
 */
Decl *decl_fun_new(const char *name, const StringList *params, const VarDeclList *locals, const CmdList *body, Expr *result_expr);

/**
 * @brief Libera a memória de um nó de declaração genérico (variável ou função).
 */
void decl_free(Decl *d);

/**
 * @brief Cria uma nova estrutura de programa.
 */
Program *program_new(void);

/**
 * @brief Adiciona uma declaração genérica (variável ou função) ao programa.
 */
void program_add_decl(Program *p, Decl *d);

/**
 * @brief Define o corpo e o retorno do bloco principal (main) do programa.
 */
void program_set_main(Program *p, const CmdList *body, Expr *result_expr);

/**
 * @brief Libera a memória de todo o programa.
 */
void program_free(Program *p);