#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "utils.h"

void parser_init(Parser *p, const char *src) {
    p->src = src;
    lexer_init(&p->lx, src);
    p->cur = lexer_next(&p->lx);
}

void parser_error_at(Parser *p, size_t pos, const char *msg) {
    size_t start = pos;
    size_t end = pos;

    while (start > 0 && p->src[start - 1] != '\n') start--;
    while (p->src[end] && p->src[end] != '\n') end++;

    fprintf(stderr, "Erro de sintaxe: %s\n", msg);
    fprintf(stderr, "Perto de pos %zu:\n", pos);
    fprintf(stderr, "  %.*s\n", (int)(end - start), p->src + start);
    fprintf(stderr, "  %*s^\n", (int)(pos - start), "");
    exit(1);
}

void advance(Parser *p) {
    token_free(&p->cur);
    p->cur = lexer_next(&p->lx);
}

void expect(Parser *p, TokenKind k, const char *what) {
    if (p->cur.kind != k) {
        char buf[160];
        snprintf(buf, sizeof(buf), "Esperava %s", what);
        parser_error_at(p, p->cur.pos, buf);
    }
}

Expr *parse_prim(Parser *p) {
    if (p->cur.kind == TOK_INT) {
        long v = p->cur.int_value;
        advance(p);
        return expr_int(v);
    }

    if (p->cur.kind == TOK_IDENT) {
        Expr *e = expr_var(p->cur.lexeme);
        advance(p);
        return e;
    }

    if (p->cur.kind == TOK_LPAREN) {
        Expr *e;
        advance(p);
        e = parse_exp_a(p);
        expect(p, TOK_RPAREN, "')'");
        advance(p);
        return e;
    }

    if (p->cur.kind == TOK_INVALID) {
        parser_error_at(p, p->cur.pos, "Caractere inválido");
    }

    parser_error_at(p, p->cur.pos, "Esperava inteiro, identificador ou '('");
    return NULL;
}

Expr *parse_exp_m(Parser *p) {
    Expr *left = parse_prim(p);

    while (p->cur.kind == TOK_OP && (p->cur.op == '*' || p->cur.op == '/')) {
        char op = p->cur.op;
        Expr *right;

        advance(p);
        right = parse_prim(p);
        left = expr_binop(op, left, right);
    }

    return left;
}

Expr *parse_exp_a(Parser *p) {
    Expr *left = parse_exp_m(p);

    while (p->cur.kind == TOK_OP && (p->cur.op == '+' || p->cur.op == '-')) {
        char op = p->cur.op;
        Expr *right;

        advance(p);
        right = parse_exp_m(p);
        left = expr_binop(op, left, right);
    }

    return left;
}

Decl *parse_decl(Parser *p) {
    char *name;
    Expr *value;
    Decl *d;

    expect(p, TOK_IDENT, "identificador");
    name = p->cur.lexeme;

    p->cur.lexeme = NULL;
    advance(p);

    expect(p, TOK_EQUAL, "'='");
    advance(p);

    value = parse_exp_a(p);

    expect(p, TOK_SEMI, "';'");
    advance(p);

    d = decl_new(name, value);
    free(name);
    return d;
}

Program *parse_program(const char *src) {
    Parser p;
    Program *program;

    parser_init(&p, src);
    program = program_new();

    while (p.cur.kind == TOK_IDENT) {
        Decl *d = parse_decl(&p);
        program_add_decl(program, d);
    }

    expect(&p, TOK_EQUAL, "'=' para iniciar a expressão final");
    advance(&p);

    program_set_result(program, parse_exp_a(&p));

    if (p.cur.kind != TOK_EOF) {
        parser_error_at(&p, p.cur.pos, "Sobrou texto depois do fim do programa");
    }

    token_free(&p.cur);
    return program;
}