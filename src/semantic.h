#pragma once

#include "ast.h"

/**
 * @brief Realiza a análise semântica do programa.
 *
 * Verifica principalmente:
 * - uso de variável antes da declaração;
 * - redeclaração de variável;
 * - atribuição a variável não declarada.
 */
void semantic_check_program(const Program *program);
