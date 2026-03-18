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

Expr *expr_int(long v) {
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    if (!e) die("Sem memória");

    e->kind = EXPR_INT;
    e->as.int_value = v;
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
        case EXPR_INT:
            break;
    }

    free(e);
}

Decl *decl_new(const char *name, Expr *value) {
    Decl *d = (Decl *)calloc(1, sizeof(Decl));
    if (!d) die("Sem memória");

    d->name = xstrdup(name);
    d->value = value;
    return d;
}

void decl_free(Decl *d) {
    if (!d) return;
    free(d->name);
    expr_free(d->value);
    free(d);
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

Program *program_new(void) {
    Program *p = (Program *)calloc(1, sizeof(Program));
    if (!p) die("Sem memória");
    cmd_list_init(&p->body);
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

void program_set_body(Program *p, const CmdList *body) {
    if (!p || !body) return;
    p->body = cmd_list_clone(body);
}

void program_set_result(Program *p, Expr *result_expr) {
    if (!p) return;
    p->result_expr = result_expr;
}

void program_free(Program *p) {
    size_t i;

    if (!p) return;

    for (i = 0; i < p->decl_count; i++) {
        decl_free(p->decls[i]);
    }

    free(p->decls);
    cmd_list_free(&p->body);
    expr_free(p->result_expr);
    free(p);
}
