#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "utils.h"

/*
 * Cria uma cópia dinâmica de uma string para que a AST seja dona
 * da memória dos nomes que armazena.
 */
static char *xstrdup(const char *s) {
    size_t n;
    char *copy;

    if (!s) return NULL;

    n = strlen(s);
    copy = (char *)malloc(n + 1);
    if (!copy) die("Sem memória");

    memcpy(copy, s, n + 1);
    return copy;
}

/* Inicializa uma lista dinâmica de expressões. */
void expr_list_init(ExprList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

/* Adiciona uma expressão ao final da lista, expandindo capacidade quando necessário. */
void expr_list_add(ExprList *list, Expr *expr) {
    Expr **new_items;
    size_t new_cap;

    if (list->count == list->capacity) {
        new_cap = (list->capacity == 0) ? 4 : list->capacity * 2;
        new_items = (Expr **)realloc(list->items, new_cap * sizeof(Expr *));
        if (!new_items) die("Sem memória");
        list->items = new_items;
        list->capacity = new_cap;
    }
    list->items[list->count++] = expr;
}

/* Cria uma nova lista com os mesmos ponteiros de expressão da lista original. */
static ExprList expr_list_clone(const ExprList *src) {
    ExprList list;
    size_t i;
    expr_list_init(&list);
    for (i = 0; i < src->count; i++) {
        expr_list_add(&list, src->items[i]);
    }
    return list;
}

/* Libera a lista de expressões e todas as expressões armazenadas nela. */
void expr_list_free(ExprList *list) {
    size_t i;
    if (!list) return;
    for (i = 0; i < list->count; i++) {
        expr_free(list->items[i]);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

/* Cria um nó de expressão inteira. */
Expr *expr_int(long v) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    if (!e) die("Sem memória");
    e->kind = EXPR_INT;
    e->as.int_value = v;
    return e;
}

/* Cria um nó de expressão booleana. */
Expr *expr_bool(int v) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    if (!e) die("Sem memória");
    e->kind = EXPR_BOOL;
    e->as.bool_value = v ? 1 : 0;
    return e;
}

/* Cria um nó de acesso a variável simples. */
Expr *expr_var(const char *name) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    if (!e) die("Sem memória");
    e->kind = EXPR_VAR;
    e->as.var_name = xstrdup(name);
    return e;
}

/* Cria um nó de operação binária. */
Expr *expr_binop(BinOpKind op, Expr *left, Expr *right) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    if (!e) die("Sem memória");
    e->kind = EXPR_BINOP;
    e->as.binop.op = op;
    e->as.binop.left = left;
    e->as.binop.right = right;
    return e;
}

/* Cria um nó de operação unária. */
Expr *expr_unop(UnOpKind op, Expr *operand) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    if (!e) die("Sem memória");
    e->kind = EXPR_UNOP;
    e->as.unop.op = op;
    e->as.unop.operand = operand;
    return e;
}

/* Cria um nó de chamada de função com cópia da lista de argumentos. */
Expr *expr_call(const char *fun_name, const ExprList *args) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    if (!e) die("Sem memória");
    e->kind = EXPR_CALL;
    e->as.call.fun_name = xstrdup(fun_name);
    e->as.call.args = expr_list_clone(args);
    return e;
}


/* Cria um nó de acesso a posição de array. */
Expr *expr_array_access(const char *name, Expr *index) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    if (!e) die("Sem memória");

    e->kind = EXPR_ARRAY_ACCESS;
    e->as.array_access.array_name = xstrdup(name);
    e->as.array_access.index = index;
    return e;
}

/* Libera recursivamente toda a memória de uma expressão. */
void expr_free(Expr *e) {
    if (!e) return;
    switch (e->kind) {
        case EXPR_VAR:
            free(e->as.var_name);
            break;
        case EXPR_BINOP:
            expr_free(e->as.binop.left);
            expr_free(e->as.binop.right);
            break;
        case EXPR_UNOP:
            expr_free(e->as.unop.operand);
            break;
        case EXPR_CALL:
            free(e->as.call.fun_name);
            expr_list_free(&e->as.call.args);
            break;
        case EXPR_ARRAY_ACCESS: 
            free(e->as.array_access.array_name);
            expr_free(e->as.array_access.index);
            break;
        case EXPR_INT:
        case EXPR_BOOL:
            break;
    }
    free(e);
}

/* Inicializa uma lista dinâmica de strings. */
void string_list_init(StringList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

/* Adiciona uma string a uma lista dinâmica. */
void string_list_add(StringList *list, char *str) {
    char **new_items;
    size_t new_cap;
    if (list->count == list->capacity) {
        new_cap = (list->capacity == 0) ? 4 : list->capacity * 2;
        new_items = (char **)realloc(list->items, new_cap * sizeof(char *));
        if (!new_items) die("Sem memória");
        list->items = new_items;
        list->capacity = new_cap;
    }
    list->items[list->count++] = str;
}

/* Duplica as strings de uma lista para uso independente na AST. */
static StringList string_list_clone(const StringList *src) {
    StringList list;
    size_t i;
    string_list_init(&list);
    for (i = 0; i < src->count; i++) {
        string_list_add(&list, xstrdup(src->items[i]));
    }
    return list;
}

/* Libera a lista de strings e o conteúdo de cada posição. */
void string_list_free(StringList *list) {
    size_t i;
    if (!list) return;
    for (i = 0; i < list->count; i++) {
        free(list->items[i]);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

/* Cria uma declaração de variável comum com valor inicial. */
VarDecl *var_decl_new(const char *name, Expr *value) {
    VarDecl *vd = (VarDecl *)calloc(1, sizeof(VarDecl));
    if (!vd) die("Sem memória");
    vd->name = xstrdup(name);
    vd->value = value;
    vd->is_array = 0;  // ALTERADO
    vd->array_size = 0;// ALTERADO
    return vd;
}


/* Cria uma declaração de array com tamanho fixo. */
VarDecl *var_decl_array_new(const char *name, size_t size) {
    VarDecl *vd = (VarDecl *)calloc(1, sizeof(VarDecl));
    if (!vd) die("Sem memória");

    vd->name = xstrdup(name);
    vd->value = NULL;
    vd->is_array = 1;
    vd->array_size = size;
    return vd;
}

/* Libera a memória de uma declaração de variável. */
void var_decl_free(VarDecl *vd) {
    if (!vd) return;
    free(vd->name);
    if (vd->value) expr_free(vd->value); // ALTERADO (pode ser NULL em array)
    free(vd);
}

/* Inicializa uma lista dinâmica de declarações de variáveis. */
void var_decl_list_init(VarDeclList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

/* Adiciona uma declaração de variável à lista. */
void var_decl_list_add(VarDeclList *list, VarDecl *vd) {
    VarDecl **new_items;
    size_t new_cap;
    if (list->count == list->capacity) {
        new_cap = (list->capacity == 0) ? 4 : list->capacity * 2;
        new_items = (VarDecl **)realloc(list->items, new_cap * sizeof(VarDecl *));
        if (!new_items) die("Sem memória");
        list->items = new_items;
        list->capacity = new_cap;
    }
    list->items[list->count++] = vd;
}

/* Cria uma cópia rasa da lista de variáveis locais. */
static VarDeclList var_decl_list_clone(const VarDeclList *src) {
    VarDeclList list;
    size_t i;
    var_decl_list_init(&list);
    for (i = 0; i < src->count; i++) {
        // Cópia rasa (apenas o ponteiro). É isso que queremos!
        var_decl_list_add(&list, src->items[i]);
    }
    return list;
}

/* Libera a lista de declarações e cada elemento armazenado. */
void var_decl_list_free(VarDeclList *list) {
    size_t i;
    if (!list) return;
    for (i = 0; i < list->count; i++) {
        var_decl_free(list->items[i]);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

/* Inicializa uma lista dinâmica de comandos. */
void cmd_list_init(CmdList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

/* Adiciona um comando à lista, realocando espaço quando necessário. */
void cmd_list_add(CmdList *list, Cmd *cmd) {
    Cmd **new_items;
    size_t new_capacity;
    if (!list || !cmd) return;
    if (list->count == list->capacity) {
        new_capacity = (list->capacity == 0) ? 4 : list->capacity * 2;
        new_items = (Cmd **)realloc(list->items, new_capacity * sizeof(Cmd *));
        if (!new_items) die("Sem memória");
        list->items = new_items;
        list->capacity = new_capacity;
    }
    list->items[list->count++] = cmd;
}

/* Cria uma cópia rasa de uma lista de comandos. */
static CmdList cmd_list_clone(const CmdList *src) {
    CmdList list;
    size_t i;
    cmd_list_init(&list);
    for (i = 0; i < src->count; i++) {
        cmd_list_add(&list, src->items[i]);
    }
    return list;
}

/* Libera a lista de comandos e cada comando nela armazenado. */
void cmd_list_free(CmdList *list) {
    size_t i;
    if (!list) return;
    for (i = 0; i < list->count; i++) {
        cmd_free(list->items[i]);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

/* Cria um comando de atribuição simples. */
Cmd *cmd_assign(const char *name, Expr *value) {
    Cmd *cmd = (Cmd *)calloc(1, sizeof(Cmd));
    if (!cmd) die("Sem memória");
    cmd->kind = CMD_ASSIGN;
    cmd->as.assign.name = xstrdup(name);
    cmd->as.assign.value = value;
    return cmd;
}

/* Cria um comando condicional com ramos then e else. */
Cmd *cmd_if(Expr *condition, const CmdList *then_branch, const CmdList *else_branch) {
    Cmd *cmd = (Cmd *)calloc(1, sizeof(Cmd));
    if (!cmd) die("Sem memória");
    cmd->kind = CMD_IF;
    cmd->as.if_cmd.condition = condition;
    cmd->as.if_cmd.then_branch = cmd_list_clone(then_branch);
    cmd->as.if_cmd.else_branch = cmd_list_clone(else_branch);
    return cmd;
}

/* Cria um comando de repetição while. */
Cmd *cmd_while(Expr *condition, const CmdList *body) {
    Cmd *cmd = (Cmd *)calloc(1, sizeof(Cmd));
    if (!cmd) die("Sem memória");
    cmd->kind = CMD_WHILE;
    cmd->as.while_cmd.condition = condition;
    cmd->as.while_cmd.body = cmd_list_clone(body);
    return cmd;
}


/* Cria um comando de atribuição em uma posição de array. */
Cmd *cmd_array_assign(const char *name, Expr *index, Expr *value) {
    Cmd *cmd = (Cmd *)calloc(1, sizeof(Cmd));
    if (!cmd) die("Sem memória");

    cmd->kind = CMD_ARRAY_ASSIGN;
    cmd->as.array_assign.name = xstrdup(name);
    cmd->as.array_assign.index = index;
    cmd->as.array_assign.value = value;
    return cmd;
}

/* Libera recursivamente a memória associada a um comando. */
void cmd_free(Cmd *cmd) {
    if (!cmd) return;
    switch (cmd->kind) {
        case CMD_ASSIGN:
            free(cmd->as.assign.name);
            expr_free(cmd->as.assign.value);
            break;
        case CMD_IF:
            expr_free(cmd->as.if_cmd.condition);
            cmd_list_free(&cmd->as.if_cmd.then_branch);
            cmd_list_free(&cmd->as.if_cmd.else_branch);
            break;
        case CMD_WHILE:
            expr_free(cmd->as.while_cmd.condition);
            cmd_list_free(&cmd->as.while_cmd.body);
            break;
        case CMD_ARRAY_ASSIGN: 
            free(cmd->as.array_assign.name);
            expr_free(cmd->as.array_assign.index);
            expr_free(cmd->as.array_assign.value);
            break;
    }
    free(cmd);
}

/* Cria uma declaração global de variável. */
Decl *decl_var_new(const char *name, Expr *value) {
    Decl *d = (Decl *)calloc(1, sizeof(Decl));
    if (!d) die("Sem memória");
    d->kind = DECL_VAR;
    d->as.var_decl.name = xstrdup(name);
    d->as.var_decl.value = value;
    d->as.var_decl.is_array = 0;  
    d->as.var_decl.array_size = 0;
    return d;
}

/* Cria uma declaração global de função. */
Decl *decl_fun_new(const char *name, const StringList *params, const VarDeclList *locals, const CmdList *body, Expr *result_expr) {
    Decl *d = (Decl *)calloc(1, sizeof(Decl));
    if (!d) die("Sem memória");
    d->kind = DECL_FUN;
    d->as.fun_decl.name = xstrdup(name);
    d->as.fun_decl.params = string_list_clone(params);
    
    d->as.fun_decl.locals = var_decl_list_clone(locals);
    
    d->as.fun_decl.body = cmd_list_clone(body);
    d->as.fun_decl.result_expr = result_expr;
    return d;
}

/* Libera uma declaração global, seja variável ou função. */
void decl_free(Decl *d) {
    size_t i;
    if (!d) return;
    if (d->kind == DECL_VAR) {
        free(d->as.var_decl.name);
        if (d->as.var_decl.value) expr_free(d->as.var_decl.value); // ALTERADO
    } else if (d->kind == DECL_FUN) {
        free(d->as.fun_decl.name);
        string_list_free(&d->as.fun_decl.params);
        for (i = 0; i < d->as.fun_decl.locals.count; i++) {
            var_decl_free(d->as.fun_decl.locals.items[i]);
        }
        free(d->as.fun_decl.locals.items);
        cmd_list_free(&d->as.fun_decl.body);
        expr_free(d->as.fun_decl.result_expr);
    }
    free(d);
}

/* Cria a estrutura principal do programa na AST. */
Program *program_new(void) {
    Program *p = (Program *)calloc(1, sizeof(Program));
    if (!p) die("Sem memória");
    cmd_list_init(&p->main_body);
    return p;
}

/* Adiciona uma declaração global ao programa. */
void program_add_decl(Program *p, Decl *d) {
    Decl **new_data;
    size_t new_capacity;
    if (!p || !d) return;
    if (p->decl_count == p->decl_capacity) {
        new_capacity = (p->decl_capacity == 0) ? 4 : p->decl_capacity * 2;
        new_data = (Decl **)realloc(p->decls, new_capacity * sizeof(Decl *));
        if (!new_data) die("Sem memória");
        p->decls = new_data;
        p->decl_capacity = new_capacity;
    }
    p->decls[p->decl_count++] = d;
}

/* Define o corpo principal e a expressão de retorno do main. */
void program_set_main(Program *p, const CmdList *main_body, Expr *main_result) {
    size_t i;
    if (!p) return;
    cmd_list_init(&p->main_body);
    for (i = 0; i < main_body->count; i++) {
        cmd_list_add(&p->main_body, main_body->items[i]);
    }
    p->main_result = main_result;
}

/* Libera toda a AST do programa. */
void program_free(Program *p) {
    size_t i;
    if (!p) return;
    for (i = 0; i < p->decl_count; i++) {
        decl_free(p->decls[i]);
    }
    free(p->decls);
    cmd_list_free(&p->main_body);
    expr_free(p->main_result);
    free(p);
}
