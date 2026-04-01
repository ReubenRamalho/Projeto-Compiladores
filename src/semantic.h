#pragma once

#include "ast.h"

/**
 * @brief Categorias de símbolos na tabela.
 */
typedef enum {
    SYM_GLOBAL_VAR,
    SYM_LOCAL_VAR,
    SYM_FUN
} SymKind;

/**
 * @brief Estrutura de um símbolo armazenado na tabela.
 */
typedef struct {
    char *name;
    SymKind kind;
    size_t arity;
} Symbol;

/**
 * @brief Tabela de símbolos dinâmica.
 */
typedef struct {
    Symbol *items;
    size_t count;
    size_t capacity;
} SymbolTable;

void semantic_check_program(const Program *program);
