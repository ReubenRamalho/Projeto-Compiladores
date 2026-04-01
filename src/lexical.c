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

static Token make_simple_token(TokenKind kind, size_t pos) {
    Token t;
    t.kind = kind;
    t.int_value = 0;
    t.lexeme = NULL;
    t.pos = pos;
    return t;
}

Token lexer_next(Lexer *lx) {
    Token t;
    char c;

    lexer_skip_ws(lx);
    c = lx->src[lx->i];

    if (c == '\0') return make_simple_token(TOK_EOF, lx->i);
    if (c == '(') { lx->i++; return make_simple_token(TOK_LPAREN, lx->i - 1); }
    if (c == ')') { lx->i++; return make_simple_token(TOK_RPAREN, lx->i - 1); }
    if (c == '{') { lx->i++; return make_simple_token(TOK_LBRACE, lx->i - 1); }
    if (c == '}') { lx->i++; return make_simple_token(TOK_RBRACE, lx->i - 1); }
    if (c == ';') { lx->i++; return make_simple_token(TOK_SEMI, lx->i - 1); }
    if (c == ',') { lx->i++; return make_simple_token(TOK_COMMA, lx->i - 1); }
    if (c == '+') { lx->i++; return make_simple_token(TOK_OP_ADD, lx->i - 1); }
    if (c == '-') { lx->i++; return make_simple_token(TOK_OP_SUB, lx->i - 1); }
    if (c == '*') { lx->i++; return make_simple_token(TOK_OP_MUL, lx->i - 1); }
    if (c == '/') { lx->i++; return make_simple_token(TOK_OP_DIV, lx->i - 1); }
    if (c == '<') {
        size_t pos = lx->i++;
        if (lx->src[lx->i] == '=') { lx->i++; return make_simple_token(TOK_OP_LE, pos); }
        return make_simple_token(TOK_OP_LT, pos);
    }
    if (c == '>') {
        size_t pos = lx->i++;
        if (lx->src[lx->i] == '=') { lx->i++; return make_simple_token(TOK_OP_GE, pos); }
        return make_simple_token(TOK_OP_GT, pos);
    }
    if (c == '=') {
        size_t pos = lx->i++;
        if (lx->src[lx->i] == '=') { lx->i++; return make_simple_token(TOK_OP_EQ, pos); }
        return make_simple_token(TOK_EQUAL, pos);
    }
    if (c == '!') {
        size_t pos = lx->i++;
        if (lx->src[lx->i] == '=') { lx->i++; return make_simple_token(TOK_OP_NE, pos); }
        return make_simple_token(TOK_INVALID, pos);
    }

    if (isalpha((unsigned char)c)) {
        size_t start = lx->i;
        char *name;
        lx->i++;
        while (isalnum((unsigned char)lx->src[lx->i]) || lx->src[lx->i] == '_') {
            lx->i++;
        }
        name = slice_dup(lx->src, start, lx->i);
        t = make_simple_token(TOK_IDENT, start);
        t.lexeme = name;

        if (strcmp(name, "if") == 0) { token_free(&t); return make_simple_token(TOK_IF, start); }
        if (strcmp(name, "else") == 0) { token_free(&t); return make_simple_token(TOK_ELSE, start); }
        if (strcmp(name, "while") == 0) { token_free(&t); return make_simple_token(TOK_WHILE, start); }
        if (strcmp(name, "return") == 0) { token_free(&t); return make_simple_token(TOK_RETURN, start); }
        if (strcmp(name, "fun") == 0) { token_free(&t); return make_simple_token(TOK_FUN, start); }
        if (strcmp(name, "var") == 0) { token_free(&t); return make_simple_token(TOK_VAR, start); }
        if (strcmp(name, "main") == 0) { token_free(&t); return make_simple_token(TOK_MAIN, start); }
        if (strcmp(name, "true") == 0) { token_free(&t); return make_simple_token(TOK_TRUE, start); }
        if (strcmp(name, "false") == 0) { token_free(&t); return make_simple_token(TOK_FALSE, start); }
        if (strcmp(name, "and") == 0) { token_free(&t); return make_simple_token(TOK_AND, start); }
        if (strcmp(name, "or") == 0) { token_free(&t); return make_simple_token(TOK_OR, start); }
        if (strcmp(name, "not") == 0) { token_free(&t); return make_simple_token(TOK_NOT, start); }
        return t;
    }

    if (isdigit((unsigned char)c)) {
        size_t start = lx->i;
        long v = 0;
        while (isdigit((unsigned char)lx->src[lx->i])) {
            int d = lx->src[lx->i] - '0';
            if (v > (LONG_MAX - d) / 10) {
                die("Inteiro muito grande perto de pos %zu", start);
            }
            v = v * 10 + d;
            lx->i++;
        }
        if (isalpha((unsigned char)lx->src[lx->i])) {
            die("Erro léxico perto de pos %zu: identificador não pode começar com dígito", start);
        }
        t = make_simple_token(TOK_INT, start);
        t.int_value = v;
        return t;
    }

    return make_simple_token(TOK_INVALID, lx->i);
}
