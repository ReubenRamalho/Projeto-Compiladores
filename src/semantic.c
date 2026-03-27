#include <stdlib.h>
#include <string.h>

#include "semantic.h"
#include "utils.h"

/**
 * @brief Inicializa a tabela de símbolos.
 */
static void symtab_init(SymbolTable *st) {
    st->items = NULL;
    st->count = 0;
    st->capacity = 0;
}

/**
 * @brief Libera a memória da tabela de símbolos e das strings dos nomes.
 */
static void symtab_free(SymbolTable *st) {
    size_t i;
    if (!st->items) return;
    for (i = 0; i < st->count; i++) {
        free(st->items[i].name);
    }
    free(st->items);
    st->items = NULL;
    st->count = 0;
    st->capacity = 0;
}

/**
 * @brief Busca um símbolo pelo nome na tabela. Retorna ponteiro para o símbolo ou NULL.
 */
static Symbol *symtab_lookup(const SymbolTable *st, const char *name) {
    size_t i;
    if (!st) return NULL;
    for (i = 0; i < st->count; i++) {
        if (strcmp(st->items[i].name, name) == 0) {
            return &st->items[i];
        }
    }
    return NULL;
}

/**
 * @brief Adiciona um novo símbolo à tabela específica. Emite erro se já existir no mesmo escopo.
 */
static void symtab_add(SymbolTable *st, const char *name, SymKind kind, size_t arity) {
    size_t new_cap;
    char *name_copy;
    size_t n;

    if (symtab_lookup(st, name)) {
        die("Erro semântico: nome '%s' já foi declarado neste escopo", name);
    }

    if (st->count == st->capacity) {
        new_cap = (st->capacity == 0) ? 8 : st->capacity * 2;
        Symbol *temp = (Symbol *)realloc(st->items, new_cap * sizeof(Symbol));
        if (!temp) die("Sem memória");
        st->items = temp;
        st->capacity = new_cap;
    }

    n = strlen(name);
    name_copy = (char *)malloc(n + 1);
    if (!name_copy) die("Sem memória");
    memcpy(name_copy, name, n + 1);

    st->items[st->count].name = name_copy;
    st->items[st->count].kind = kind;
    st->items[st->count].arity = arity;
    st->count++;
}

/* Declaração antecipada das funções de checagem mútua */
static void check_expr(const Expr *e, SymbolTable *global_st, SymbolTable *local_st);
static void check_cmd_list(const CmdList *list, SymbolTable *global_st, SymbolTable *local_st);

/**
 * @brief Verifica a semântica de uma expressão, buscando em escopos locais e globais.
 */
static void check_expr(const Expr *e, SymbolTable *global_st, SymbolTable *local_st) {
    Symbol *sym;
    size_t i;

    if (!e) return;

    switch (e->kind) {
        case EXPR_INT:
            return;
            
        case EXPR_VAR:
            // Regra de Shadowing: Busca local primeiro, depois global
            sym = symtab_lookup(local_st, e->as.var_name);
            if (!sym) {
                sym = symtab_lookup(global_st, e->as.var_name);
            }
            
            if (!sym) {
                die("Erro semântico: variável '%s' não declarada", e->as.var_name);
            }
            if (sym->kind == SYM_FUN) {
                die("Erro semântico: '%s' é uma função, mas está sendo usada como variável", e->as.var_name);
            }
            return;
            
        case EXPR_BINOP:
            check_expr(e->as.binop.left, global_st, local_st);
            check_expr(e->as.binop.right, global_st, local_st);
            return;
            
        case EXPR_CALL:
            // Funções só existem no escopo global
            sym = symtab_lookup(global_st, e->as.call.fun_name);
            if (!sym) {
                die("Erro semântico: função '%s' não declarada", e->as.call.fun_name);
            }
            if (sym->kind != SYM_FUN) {
                die("Erro semântico: '%s' não é uma função e não pode ser chamada", e->as.call.fun_name);
            }
            if (sym->arity != e->as.call.args.count) {
                die("Erro semântico: a função '%s' espera %zu argumentos, mas %zu foram passados", 
                    e->as.call.fun_name, sym->arity, e->as.call.args.count);
            }
            
            // Verifica as expressões passadas como argumentos
            for (i = 0; i < e->as.call.args.count; i++) {
                check_expr(e->as.call.args.items[i], global_st, local_st);
            }
            return;
    }
}

/**
 * @brief Verifica a semântica de um comando.
 */
static void check_cmd(const Cmd *cmd, SymbolTable *global_st, SymbolTable *local_st) {
    Symbol *sym;

    switch (cmd->kind) {
        case CMD_ASSIGN:
            sym = symtab_lookup(local_st, cmd->as.assign.name);
            if (!sym) {
                sym = symtab_lookup(global_st, cmd->as.assign.name);
            }
            
            if (!sym) {
                die("Erro semântico: variável '%s' não foi declarada antes da atribuição", cmd->as.assign.name);
            }
            if (sym->kind == SYM_FUN) {
                die("Erro semântico: impossível atribuir valor à função '%s'", cmd->as.assign.name);
            }
            check_expr(cmd->as.assign.value, global_st, local_st);
            return;
            
        case CMD_IF:
            check_expr(cmd->as.if_cmd.condition, global_st, local_st);
            check_cmd_list(&cmd->as.if_cmd.then_branch, global_st, local_st);
            check_cmd_list(&cmd->as.if_cmd.else_branch, global_st, local_st);
            return;
            
        case CMD_WHILE:
            check_expr(cmd->as.while_cmd.condition, global_st, local_st);
            check_cmd_list(&cmd->as.while_cmd.body, global_st, local_st);
            return;
    }
}

/**
 * @brief Itera sobre uma lista de comandos verificando sua semântica.
 */
static void check_cmd_list(const CmdList *list, SymbolTable *global_st, SymbolTable *local_st) {
    size_t i;
    for (i = 0; i < list->count; i++) {
        check_cmd(list->items[i], global_st, local_st);
    }
}

/**
 * @brief Ponto de entrada da análise semântica.
 * Processa as declarações globais (permitindo recursão) e verifica corpos de funções e o main.
 */
void semantic_check_program(const Program *program) {
    SymbolTable global_st;
    size_t i, j;

    symtab_init(&global_st);

    // Passo 1: Analisa as declarações globais em ordem
    for (i = 0; i < program->decl_count; i++) {
        const Decl *d = program->decls[i];
        
        if (d->kind == DECL_VAR) {
            // A expressão da variável global só enxerga o que já está na tabela global
            check_expr(d->as.var_decl.value, &global_st, NULL);
            symtab_add(&global_st, d->as.var_decl.name, SYM_GLOBAL_VAR, 0);
        } 
        else if (d->kind == DECL_FUN) {
            SymbolTable local_st;
            
            // 1. Registra a função no escopo global PRIMEIRO, para permitir recursão direta
            symtab_add(&global_st, d->as.fun_decl.name, SYM_FUN, d->as.fun_decl.params.count);
            
            // 2. Inicializa o escopo local da função
            symtab_init(&local_st);
            
            // 3. Adiciona os parâmetros na tabela local
            for (j = 0; j < d->as.fun_decl.params.count; j++) {
                symtab_add(&local_st, d->as.fun_decl.params.items[j], SYM_LOCAL_VAR, 0);
            }
            
            // 4. Analisa e adiciona as variáveis locais da função
            for (j = 0; j < d->as.fun_decl.locals.count; j++) {
                const VarDecl *local_var = d->as.fun_decl.locals.items[j];
                check_expr(local_var->value, &global_st, &local_st);
                symtab_add(&local_st, local_var->name, SYM_LOCAL_VAR, 0);
            }
            
            // 5. Verifica os comandos do corpo e a expressão de retorno
            check_cmd_list(&d->as.fun_decl.body, &global_st, &local_st);
            check_expr(d->as.fun_decl.result_expr, &global_st, &local_st);
            
            // 6. Destrói o escopo local ao terminar a função
            symtab_free(&local_st);
        }
    }

    // Passo 2: Verifica o bloco main (só enxerga o escopo global)
    check_cmd_list(&program->main_body, &global_st, NULL);
    check_expr(program->main_result, &global_st, NULL);

    symtab_free(&global_st);
}
