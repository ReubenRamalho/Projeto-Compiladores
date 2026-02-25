#include <ctype.h>
#include <limits.h>

#include "lexical.h"
#include "utils.h"

void lexer_init(Lexer *lx, const char *src) {
    lx->src = src;
    lx->i = 0;
}

void lexer_skip_ws(Lexer *lx) {
    // Pula espaço, quebra de linha, tab, etc
    while (lx->src[lx->i] && isspace((unsigned char)lx->src[lx->i])) lx->i++;
}

Token lexer_next(Lexer *lx) {
    lexer_skip_ws(lx);

    Token t;
    t.pos = lx->i;
    t.int_value = 0;
    t.op = 0;

    char c = lx->src[lx->i];
    if (c == '\0') {
        t.kind = TOK_EOF; // Fim do arquivo
        return t;
    }

    // Identifica operador ou parênteses de boas
    if (c == '(') {
        lx->i++;
        t.kind = TOK_LPAREN;
        return t;
    }
    if (c == ')') {
        lx->i++;
        t.kind = TOK_RPAREN;
        return t;
    }
    if (c == '+' || c == '-' || c == '*' || c == '/') {
        lx->i++;
        t.kind = TOK_OP;
        t.op = c;
        return t;
    }

    // Lida com números grandes
    if (isdigit((unsigned char)c)) {
        long v = 0;
        while (isdigit((unsigned char)lx->src[lx->i])) {
            int d = lx->src[lx->i] - '0'; // Converte o char pra int
            
            // Trava de segurança pra variável não dar overflow na memória
            if (v > (LONG_MAX - d) / 10) die("Inteiro muito grande perto de pos %zu", t.pos);
            
            v = v * 10 + d; // Vai juntando os dígitos: 1 -> 10 -> 105...
            lx->i++;
        }
        t.kind = TOK_INT;
        t.int_value = v;
        return t;
    }

    // Se chegou aqui e não era nada disso, lascou
    t.kind = TOK_INVALID;
    return t;
}