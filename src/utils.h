#pragma once

#include <stdio.h>

/**
 * @brief Encerra o programa exibindo uma mensagem de erro formatada.
 */
void die(const char *fmt, ...);

/**
 * @brief Lê todo o conteúdo de um arquivo e retorna uma string alocada dinamicamente.
 */
char *read_entire_file(const char *path);

/**
 * @brief Emite uma linha formatada para o arquivo de saída.
 */
void emit(FILE *out, const char *fmt, ...);
