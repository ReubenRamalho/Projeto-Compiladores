#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "lexical.h"
#include "utils.h"

static char *slice_dup(const char *src, size_t start, size_t end) {
    size_t len = end - start;
    char *s = (char *)malloc(len + 1);
    if (!s) die("Sem memória");

    memcpy(s, src + start, len);
    s[len] = '\0';
    return s;
}

void lexer_init(Lexer *lx, const char *src) {
    lx->src = src;
    lx->i = 0;
}

void lexer_skip_ws(Lexer *lx) {
    while (lx->src[lx->i] && isspace((unsigned char)lx->src[lx->i])) {
        lx->i++;
    }
}

void token_free(Token *t) {
    if (!t) return;
    free(t->lexeme);
    t->lexeme = NULL;
}

Token lexer_next(Lexer *lx) {
    Token t;
    char c;

    lexer_skip_ws(lx);

    t.kind = TOK_INVALID;
    t.int_value = 0;
    t.op = 0;
    t.lexeme = NULL;
    t.pos = lx->i;

    c = lx->src[lx->i];

    if (c == '\0') {
        t.kind = TOK_EOF;
        return t;
    }

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

    if (c == '=') {
        lx->i++;
        t.kind = TOK_EQUAL;
        return t;
    }

    if (c == ';') {
        lx->i++;
        t.kind = TOK_SEMI;
        return t;
    }

    if (isalpha((unsigned char)c)) {
        size_t start = lx->i;
        lx->i++;

        while (isalnum((unsigned char)lx->src[lx->i])) {
            lx->i++;
        }

        t.kind = TOK_IDENT;
        t.lexeme = slice_dup(lx->src, start, lx->i);
        return t;
    }

    if (isdigit((unsigned char)c)) {
        size_t start = lx->i;
        long v = 0;

        while (isdigit((unsigned char)lx->src[lx->i])) {
            int d = lx->src[lx->i] - '0';

            if (v > (LONG_MAX - d) / 10) {
                die("Inteiro muito grande perto de pos %zu", t.pos);
            }

            v = v * 10 + d;
            lx->i++;
        }

        if (isalpha((unsigned char)lx->src[lx->i])) {
            die("Erro léxico perto de pos %zu: identificador não pode começar com dígito", start);
        }

        t.kind = TOK_INT;
        t.int_value = v;
        return t;
    }

    t.kind = TOK_INVALID;
    return t;
}