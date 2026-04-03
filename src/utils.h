#pragma once

/*
 * Funções auxiliares compartilhadas entre os módulos do compilador.
 */

#include <stdio.h>

/* Interrompe a execução com mensagem de erro. */
void die(const char *fmt, ...);
/* Lê um arquivo inteiro para memória. */
char *read_entire_file(const char *path);
/* Escreve uma linha formatada em um arquivo. */
void emit(FILE *out, const char *fmt, ...);
