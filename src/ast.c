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

Expr *expr_binop(char op, Expr *left, Expr *right) {
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

Program *program_new(void) {
    Program *p = (Program *)calloc(1, sizeof(Program));
    if (!p) die("Sem memória");
    return p;
}

void program_add_decl(Program *p, Decl *d) {
    if (!p || !d) return;

    if (p->decl_count == p->decl_capacity) {
        size_t new_capacity = (p->decl_capacity == 0) ? 4 : p->decl_capacity * 2;
        Decl **new_data = (Decl **)realloc(p->decls, new_capacity * sizeof(Decl *));
        if (!new_data) die("Sem memória");

        p->decls = new_data;
        p->decl_capacity = new_capacity;
    }

    p->decls[p->decl_count++] = d;
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
    expr_free(p->result_expr);
    free(p);
}