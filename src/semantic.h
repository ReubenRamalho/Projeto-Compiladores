#pragma once

#include "ast.h"

/*
 * Tabela de símbolos simples baseada em vetor dinâmico.
 * Para esta atividade, basta guardar os nomes declarados.
 */
typedef struct {
    char **names;
    size_t count;
    size_t capacity;
} SymbolTable;


/**
 * @brief Realiza a análise semântica do programa.
 *
 * Verifica principalmente:
 *   - uso de variável antes da declaração
 *   - redeclaração de variável
 */
void semantic_check_program(const Program *program);