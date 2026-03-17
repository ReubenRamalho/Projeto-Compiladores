#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "semantic.h"
#include "utils.h"


static void symtab_init(SymbolTable *st) {
    st->names = NULL;
    st->count = 0;
    st->capacity = 0;
}

static void symtab_free(SymbolTable *st) {
    size_t i;
    for (i = 0; i < st->count; i++) {
        free(st->names[i]);
    }
    free(st->names);
}

static int symtab_contains(const SymbolTable *st, const char *name) {
    size_t i;
    for (i = 0; i < st->count; i++) {
        if (strcmp(st->names[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

static void symtab_add(SymbolTable *st, const char *name) {
    char *copy;
    size_t n;

    if (symtab_contains(st, name)) {
        die("Erro semântico: variável '%s' declarada mais de uma vez", name);
    }

    if (st->count == st->capacity) {
        size_t new_capacity = (st->capacity == 0) ? 8 : st->capacity * 2;
        char **new_names = (char **)realloc(st->names, new_capacity * sizeof(char *));
        if (!new_names) die("Sem memória");
        st->names = new_names;
        st->capacity = new_capacity;
    }

    n = strlen(name);
    copy = (char *)malloc(n + 1);
    if (!copy) die("Sem memória");

    memcpy(copy, name, n + 1);
    st->names[st->count++] = copy;
}

static void check_expr(const Expr *e, const SymbolTable *st) {
    if (!e) return;

    switch (e->kind) {
        case EXPR_INT:
            return;

        case EXPR_VAR:
            if (!symtab_contains(st, e->as.var_name)) {
                die("Erro semântico: variável '%s' usada antes da declaração", e->as.var_name);
            }
            return;

        case EXPR_BINOP:
            check_expr(e->as.binop.left, st);
            check_expr(e->as.binop.right, st);
            return;

        default:
            die("Erro interno: expressão desconhecida na análise semântica");
    }
}

void semantic_check_program(const Program *program) {
    SymbolTable st;
    size_t i;

    symtab_init(&st);

    for (i = 0; i < program->decl_count; i++) {
        const Decl *d = program->decls[i];

        check_expr(d->value, &st);
        symtab_add(&st, d->name);
    }

    check_expr(program->result_expr, &st);
    symtab_free(&st);
}