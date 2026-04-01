#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "utils.h"

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

void expr_list_init(ExprList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

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

static ExprList expr_list_clone(const ExprList *src) {
    ExprList list;
    size_t i;
    expr_list_init(&list);
    for (i = 0; i < src->count; i++) {
        expr_list_add(&list, src->items[i]);
    }
    return list;
}

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

Expr *expr_int(long v) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    if (!e) die("Sem memória");
    e->kind = EXPR_INT;
    e->as.int_value = v;
    return e;
}

Expr *expr_bool(int v) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    if (!e) die("Sem memória");
    e->kind = EXPR_BOOL;
    e->as.bool_value = v ? 1 : 0;
    return e;
}

Expr *expr_var(const char *name) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    if (!e) die("Sem memória");
    e->kind = EXPR_VAR;
    e->as.var_name = xstrdup(name);
    return e;
}

Expr *expr_binop(BinOpKind op, Expr *left, Expr *right) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    if (!e) die("Sem memória");
    e->kind = EXPR_BINOP;
    e->as.binop.op = op;
    e->as.binop.left = left;
    e->as.binop.right = right;
    return e;
}

Expr *expr_unop(UnOpKind op, Expr *operand) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    if (!e) die("Sem memória");
    e->kind = EXPR_UNOP;
    e->as.unop.op = op;
    e->as.unop.operand = operand;
    return e;
}

Expr *expr_call(const char *fun_name, const ExprList *args) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    if (!e) die("Sem memória");
    e->kind = EXPR_CALL;
    e->as.call.fun_name = xstrdup(fun_name);
    e->as.call.args = expr_list_clone(args);
    return e;
}

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
        case EXPR_INT:
        case EXPR_BOOL:
            break;
    }
    free(e);
}

void string_list_init(StringList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

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

static StringList string_list_clone(const StringList *src) {
    StringList list;
    size_t i;
    string_list_init(&list);
    for (i = 0; i < src->count; i++) {
        string_list_add(&list, xstrdup(src->items[i]));
    }
    return list;
}

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

VarDecl *var_decl_new(const char *name, Expr *value) {
    VarDecl *vd = (VarDecl *)calloc(1, sizeof(VarDecl));
    if (!vd) die("Sem memória");
    vd->name = xstrdup(name);
    vd->value = value;
    return vd;
}

void var_decl_free(VarDecl *vd) {
    if (!vd) return;
    free(vd->name);
    expr_free(vd->value);
    free(vd);
}

void var_decl_list_init(VarDeclList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

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

static VarDeclList var_decl_list_clone(const VarDeclList *src) {
    VarDeclList list;
    size_t i;
    var_decl_list_init(&list);
    for (i = 0; i < src->count; i++) {
        var_decl_list_add(&list, var_decl_new(src->items[i]->name, src->items[i]->value));
    }
    return list;
}

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

void cmd_list_init(CmdList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

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

static CmdList cmd_list_clone(const CmdList *src) {
    CmdList list;
    size_t i;
    cmd_list_init(&list);
    for (i = 0; i < src->count; i++) {
        cmd_list_add(&list, src->items[i]);
    }
    return list;
}

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

Cmd *cmd_assign(const char *name, Expr *value) {
    Cmd *cmd = (Cmd *)calloc(1, sizeof(Cmd));
    if (!cmd) die("Sem memória");
    cmd->kind = CMD_ASSIGN;
    cmd->as.assign.name = xstrdup(name);
    cmd->as.assign.value = value;
    return cmd;
}

Cmd *cmd_if(Expr *condition, const CmdList *then_branch, const CmdList *else_branch) {
    Cmd *cmd = (Cmd *)calloc(1, sizeof(Cmd));
    if (!cmd) die("Sem memória");
    cmd->kind = CMD_IF;
    cmd->as.if_cmd.condition = condition;
    cmd->as.if_cmd.then_branch = cmd_list_clone(then_branch);
    cmd->as.if_cmd.else_branch = cmd_list_clone(else_branch);
    return cmd;
}

Cmd *cmd_while(Expr *condition, const CmdList *body) {
    Cmd *cmd = (Cmd *)calloc(1, sizeof(Cmd));
    if (!cmd) die("Sem memória");
    cmd->kind = CMD_WHILE;
    cmd->as.while_cmd.condition = condition;
    cmd->as.while_cmd.body = cmd_list_clone(body);
    return cmd;
}

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
    }
    free(cmd);
}

Decl *decl_var_new(const char *name, Expr *value) {
    Decl *d = (Decl *)calloc(1, sizeof(Decl));
    if (!d) die("Sem memória");
    d->kind = DECL_VAR;
    d->as.var_decl.name = xstrdup(name);
    d->as.var_decl.value = value;
    return d;
}

Decl *decl_fun_new(const char *name, const StringList *params, const VarDeclList *locals, const CmdList *body, Expr *result_expr) {
    Decl *d = (Decl *)calloc(1, sizeof(Decl));
    size_t i;
    if (!d) die("Sem memória");
    d->kind = DECL_FUN;
    d->as.fun_decl.name = xstrdup(name);
    d->as.fun_decl.params = string_list_clone(params);
    var_decl_list_init(&d->as.fun_decl.locals);
    for (i = 0; i < locals->count; i++) {
        var_decl_list_add(&d->as.fun_decl.locals, var_decl_new(locals->items[i]->name, locals->items[i]->value));
    }
    d->as.fun_decl.body = cmd_list_clone(body);
    d->as.fun_decl.result_expr = result_expr;
    return d;
}

void decl_free(Decl *d) {
    size_t i;
    if (!d) return;
    if (d->kind == DECL_VAR) {
        free(d->as.var_decl.name);
        expr_free(d->as.var_decl.value);
    } else {
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

Program *program_new(void) {
    Program *p = (Program *)calloc(1, sizeof(Program));
    if (!p) die("Sem memória");
    cmd_list_init(&p->main_body);
    return p;
}

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

void program_set_main(Program *p, const CmdList *main_body, Expr *main_result) {
    size_t i;
    if (!p) return;
    cmd_list_init(&p->main_body);
    for (i = 0; i < main_body->count; i++) {
        cmd_list_add(&p->main_body, main_body->items[i]);
    }
    p->main_result = main_result;
}

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
