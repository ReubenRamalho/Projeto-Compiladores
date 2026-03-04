#include <stdlib.h>

#include "parser.h"
#include "ast.h"

void parser_init(Parser *p, const char *src) {
    p->src = src;
    lexer_init(&p->lx, src);
    p->cur = lexer_next(&p->lx); // Já puxa o primeiro token pra começar os trabalhos
}

void parser_error_at(Parser *p, size_t pos, const char *msg) {
    size_t start = pos;
    while (start > 0 && p->src[start - 1] != '\n') start--; // Acha o começo da linha do erro

    size_t end = pos;
    while (p->src[end] && p->src[end] != '\n') end++; // Acha o fim da linha

    // Cospe o erro formatado bonitinho mostrando onde quebrou
    fprintf(stderr, "Erro de sintaxe: %s\n", msg);
    fprintf(stderr, "Perto de pos %zu:\n", pos);
    fprintf(stderr, "  %.*s\n", (int)(end - start), p->src + start);
    fprintf(stderr, "  %*s^\n", (int)(pos - start), "");

    exit(1); // Morreu aqui.
}

void expect(Parser *p, TokenKind k, const char *what) {
    if (p->cur.kind != k) {
        char buf[160];
        snprintf(buf, sizeof(buf), "Esperava %s", what);
        parser_error_at(p, p->cur.pos, buf); // Se não for o token que a gente queria, capota o programa
    }
}

void advance(Parser *p) {
    p->cur = lexer_next(&p->lx); // Puxa o próximo token da fita
}

Node *parse_prim(Parser *p) {
    if (p->cur.kind == TOK_INT) {
        long v = p->cur.int_value;
        advance(p);
        return node_int(v); // Achou número? Vira nó folha da árvore e pronto
    }

    if (p->cur.kind == TOK_LPAREN) {
        advance(p); // Engole o '('
        
        Node *expr = parse_exp_a(p); // Volta pro topo da prioridade pra resolver o que tá dentro

        expect(p, TOK_RPAREN, "')'"); // Garante que fecharam o parênteses
        advance(p); // Engole o ')'

        return expr;
    }

    if (p->cur.kind == TOK_INVALID) {
        parser_error_at(p, p->cur.pos, "Caractere inválido");
    }

    parser_error_at(p, p->cur.pos, "Esperava inteiro ou '('");
    return NULL;
}

Node *parse_exp_m(Parser *p) {
    Node *left = parse_prim(p); // Tenta achar um número ou parênteses primeiro

    // Enquanto tiver * ou /, a gente vai empilhando na árvore
    while (p->cur.kind == TOK_OP && (p->cur.op == '*' || p->cur.op == '/')) {
        char op = p->cur.op;
        advance(p); // Consome o * ou /
        
        Node *right = parse_prim(p); // Pega o número do lado direito
        
        // O pulo do gato: o que já tava na esquerda vira filho da nova operação
        left = node_binop(op, left, right); 
    }

    return left;
}

Node *parse_exp_a(Parser *p) {
    Node *left = parse_exp_m(p); // Tenta resolver as multiplicações primeiro (precedência!)

    // Enquanto tiver + ou -, o esquema é igualzinho ao da multiplicação
    while (p->cur.kind == TOK_OP && (p->cur.op == '+' || p->cur.op == '-')) {
        char op = p->cur.op;
        advance(p);
        
        Node *right = parse_exp_m(p);
        
        // Esse while salva a gente de tomar um loop infinito de recursão
        left = node_binop(op, left, right); 
    }

    return left;
}

Node *parse_program(const char *src) {
    Parser p;
    parser_init(&p, src);

    Node *ast = parse_exp_a(&p); // Começa sempre do nível mais baixo (soma/subtração)

    if (p.cur.kind != TOK_EOF) {
        // Se a gramática terminou mas ainda tem texto solto no arquivo, tá errado
        parser_error_at(&p, p.cur.pos, "Sobrou texto depois do fim da expressão");
    }

    return ast;
}