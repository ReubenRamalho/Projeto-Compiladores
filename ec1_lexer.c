/*
 * EC1 Lexer (Expressões Constantes 1) - Atividade 04
 *
 * Lê um arquivo contendo uma expressão EC1 e realiza a análise léxica,
 * imprimindo a sequência de tokens encontrados no formato:
 * <Tipo, "Lexema", Posicao>
 */

#include <stdio.h>
#include <stdlib.h>

#include "src/lexical.h"
#include "src/utils.h"


static void print_token(const Token *t) {
    switch (t->kind) {
        case TOK_INT:
            printf("<Numero, \"%ld\", %zu>\n", t->int_value, t->pos);
            break;
        case TOK_LPAREN:
            printf("<ParenEsq, \"(\", %zu>\n", t->pos);
            break;
        case TOK_RPAREN:
            printf("<ParenDir, \")\", %zu>\n", t->pos);
            break;
        case TOK_OP:
            if (t->op == '+') printf("<Soma, \"+\", %zu>\n", t->pos);
            else if (t->op == '-') printf("<Sub, \"-\", %zu>\n", t->pos);
            else if (t->op == '*') printf("<Mult, \"*\", %zu>\n", t->pos);
            else if (t->op == '/') printf("<Div, \"/\", %zu>\n", t->pos);
            break;
        case TOK_INVALID:
            break;
        case TOK_EOF:
            break;
    }
}

/* =========================
 * Main
 * ========================= */

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s arquivo.ci\n", argv[0]);
        return 1;
    }

    char *src = read_entire_file(argv[1]);

    Lexer lx;
    lexer_init(&lx, src);

    Token t;
    do {
        t = lexer_next(&lx);
        
        if (t.kind == TOK_INVALID) {
            fprintf(stderr, "Erro léxico na posição %zu\n", t.pos);
            free(src);
            return 1; 
        }
        
        if (t.kind != TOK_EOF) {
            print_token(&t);
        }
    } while (t.kind != TOK_EOF);

    free(src);
    return 0;
}