#include <stdlib.h>
#include <string.h>

#include "semantic.h"
#include "utils.h"

/**
 * @brief Tabela de símbolos simples para variáveis declaradas.
 */
typedef struct {
    char **names;
    size_t count;
    size_t capacity;
} SymbolTable;

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
    char **new_names;
    char *copy;
    size_t new_capacity;
    size_t n;

    if (symtab_contains(st, name)) {
        die("Erro semântico: variável '%s' declarada mais de uma vez", name);
    }

    if (st->count == st->capacity) {
        new_capacity = (st->capacity == 0) ? 8 : st->capacity * 2;
        new_names = (char **)realloc(st->names, new_capacity * sizeof(char *));
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
    }
}

static void check_cmd_list(const CmdList *list, const SymbolTable *st);

static void check_cmd(const Cmd *cmd, const SymbolTable *st) {
    switch (cmd->kind) {
        case CMD_ASSIGN:
            if (!symtab_contains(st, cmd->as.assign.name)) {
                die("Erro semântico: variável '%s' não foi declarada antes da atribuição", cmd->as.assign.name);
            }
            check_expr(cmd->as.assign.value, st);
            return;
        case CMD_IF:
            check_expr(cmd->as.if_cmd.condition, st);
            check_cmd_list(&cmd->as.if_cmd.then_branch, st);
            check_cmd_list(&cmd->as.if_cmd.else_branch, st);
            return;
        case CMD_WHILE:
            check_expr(cmd->as.while_cmd.condition, st);
            check_cmd_list(&cmd->as.while_cmd.body, st);
            return;
    }
}

static void check_cmd_list(const CmdList *list, const SymbolTable *st) {
    size_t i;
    for (i = 0; i < list->count; i++) {
        check_cmd(list->items[i], st);
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

    check_cmd_list(&program->body, &st);
    check_expr(program->result_expr, &st);
    symtab_free(&st);
}
