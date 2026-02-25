#include <stdlib.h>

#include "parser.h"
#include "ast.h"

void parser_init(Parser *p, const char *src) {
    p->src = src;
    lexer_init(&p->lx, src);
    p->cur = lexer_next(&p->lx);
}

void parser_error_at(Parser *p, size_t pos, const char *msg) {
    size_t start = pos;
    while (start > 0 && p->src[start - 1] != '\n') start--;

    size_t end = pos;
    while (p->src[end] && p->src[end] != '\n') end++;

    fprintf(stderr, "Erro de sintaxe: %s\n", msg);
    fprintf(stderr, "Perto de pos %zu:\n", pos);
    fprintf(stderr, "  %.*s\n", (int)(end - start), p->src + start);
    fprintf(stderr, "  %*s^\n", (int)(pos - start), "");

    exit(1);
}


void expect(Parser *p, TokenKind k, const char *what) {
    if (p->cur.kind != k) {
        char buf[160];
        snprintf(buf, sizeof(buf), "Esperava %s", what);
        parser_error_at(p, p->cur.pos, buf);
    }
}

void advance(Parser *p) {
    p->cur = lexer_next(&p->lx);
}

Node *parse_prim(Parser *p) {
    if (p->cur.kind == TOK_INT) {
        long v = p->cur.int_value;
        advance(p);
        return node_int(v);
    }

    if (p->cur.kind == TOK_LPAREN) {
        advance(p);
        
        Node *expr = parse_exp_a(p); 

        expect(p, TOK_RPAREN, "')'");
        advance(p);

        return expr;
    }

    if (p->cur.kind == TOK_INVALID) {
        parser_error_at(p, p->cur.pos, "Caractere inválido");
    }

    parser_error_at(p, p->cur.pos, "Esperava inteiro ou '('");
    return NULL;
}

Node *parse_exp_m(Parser *p) {
    Node *left = parse_prim(p);

    while (p->cur.kind == TOK_OP && (p->cur.op == '*' || p->cur.op == '/')) {
        char op = p->cur.op;
        advance(p);
        
        Node *right = parse_prim(p);
        
        left = node_binop(op, left, right); 
    }

    return left;
}

Node *parse_exp_a(Parser *p) {
    Node *left = parse_exp_m(p); 

    while (p->cur.kind == TOK_OP && (p->cur.op == '+' || p->cur.op == '-')) {
        char op = p->cur.op;
        advance(p);
        
        Node *right = parse_exp_m(p);
        
        left = node_binop(op, left, right); 
    }

    return left;
}

Node *parse_program(const char *src) {
    Parser p;
    parser_init(&p, src);

    Node *ast = parse_exp_a(&p);

    if (p.cur.kind != TOK_EOF) {
        parser_error_at(&p, p.cur.pos, "Sobrou texto depois do fim da expressão");
    }

    return ast;
}