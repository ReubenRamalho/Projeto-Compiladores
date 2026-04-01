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
    int is_array;
} Symbol;

/**
 * @brief Tabela de símbolos dinâmica.
 */
typedef struct {
    Symbol *items;
    size_t count;
    size_t capacity;
} SymbolTable;


/**
 * @brief Realiza a análise semântica do programa Fun.
 *
 * Verifica:
 * - uso de variável antes da declaração (respeitando escopo local e global);
 * - redeclaração de variáveis no mesmo escopo;
 * - chamadas a funções não declaradas ou com número incorreto de argumentos;
 * - se uma variável está tentando ser chamada como função ou vice-versa.
 */
void semantic_check_program(const Program *program);
