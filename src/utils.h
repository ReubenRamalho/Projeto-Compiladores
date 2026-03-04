#pragma once

#include <stdio.h>

/**
 * @brief Encerra o programa exibindo uma mensagem de erro formatada.
 * @param fmt String de formato (como printf).
 * @param ... Argumentos variádicos para a string de formato.
 */
void die(const char *fmt, ...);

/**
 * @brief Lê todo o conteúdo de um arquivo e retorna como uma string.
 * @param path Caminho para o arquivo.
 * @return Ponteiro para o conteúdo lido (deve ser liberado pelo usuário).
 */
char *read_entire_file(const char *path);

/**
 * @brief Emite uma mensagem formatada para um arquivo de saída.
 * @param out Ponteiro para o arquivo de saída.
 * @param fmt String de formato (como printf).
 * @param ... Argumentos variádicos para a string de formato.
 */
void emit(FILE *out, const char *fmt, ...);