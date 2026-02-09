/*
 * EC1 Compiler (Expressões Constantes 1)
 *
 * Lê um arquivo .ci contendo uma expressão EC1, constrói uma AST e gera um output.s
 * que calcula a expressão e imprime usando imprime_num/sair do runtime.s.
 *
 * Gramática EC1:
 *   <programa> ::= <expressao>
 *   <expressao> ::= <literal-inteiro> | '(' <expressao> <operador> <expressao> ')'
 *   <operador> ::= '+' | '-' | '*' | '/'
 *   <literal-inteiro> ::= <digito>+
 *
 * Geração de código:
 *   - Resultado final SEMPRE em %rax
 *   - Estratégia com pilha (push/pop) recomendada para evitar “falta de registradores”
 *     e preservar a ordem correta em '-' e '/'.
 *
 * Saída (output.s):
 *   .section .text
 *   .globl _start
 *   _start:
 *      <codigo da expr>
 *      call imprime_num
 *      call sair
 *      .include "runtime.s"
 *
 * Observação: Se você usar .include "runtime.s", NÃO linke runtime.o separado.
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================
 *  Erros e utilitários
 * ========================= */

static void die(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    va_end(args);
    exit(1);
}

static char *read_entire_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) die("Erro abrindo '%s': %s", path, strerror(errno));

    if (fseek(f, 0, SEEK_END) != 0) die("fseek falhou");
    long n = ftell(f);
    if (n < 0) die("ftell falhou");
    rewind(f);

    char *buf = (char *)malloc((size_t)n + 1);
    if (!buf) die("Sem memória");

    size_t got = fread(buf, 1, (size_t)n, f);
    fclose(f);

    if (got != (size_t)n) die("Leitura incompleta");
    buf[n] = '\0';
    return buf;
}

/* =========================
 *  Léxico (tokenização)
 * ========================= */

typedef enum {
    TOK_INT,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_OP,
    TOK_EOF,
    TOK_INVALID
} TokenKind;

typedef struct {
    TokenKind kind;
    long      int_value; /* se TOK_INT */
    char      op;        /* se TOK_OP  */
    size_t    pos;       /* índice no texto */
} Token;

typedef struct {
    const char *src;
    size_t i;
} Lexer;

static void lexer_init(Lexer *lx, const char *src) {
    lx->src = src;
    lx->i = 0;
}

static void lexer_skip_ws(Lexer *lx) {
    while (lx->src[lx->i] && isspace((unsigned char)lx->src[lx->i])) lx->i++;
}

static Token lexer_next(Lexer *lx) {
    lexer_skip_ws(lx);

    Token t;
    t.pos = lx->i;
    t.int_value = 0;
    t.op = 0;

    char c = lx->src[lx->i];
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

    if (isdigit((unsigned char)c)) {
        long v = 0;
        while (isdigit((unsigned char)lx->src[lx->i])) {
            int d = lx->src[lx->i] - '0';
            if (v > (LONG_MAX - d) / 10) die("Inteiro muito grande perto de pos %zu", t.pos);
            v = v * 10 + d;
            lx->i++;
        }
        t.kind = TOK_INT;
        t.int_value = v;
        return t;
    }

    t.kind = TOK_INVALID;
    return t;
}

/* =========================
 *  AST
 * ========================= */

typedef enum { NODE_INT, NODE_BINOP } NodeKind;

typedef struct Node {
    NodeKind kind;
    union {
        long int_value;
        struct {
            char op;
            struct Node *left;
            struct Node *right;
        } binop;
    } as;
} Node;

static Node *node_int(long v) {
    Node *n = (Node *)calloc(1, sizeof(Node));
    if (!n) die("Sem memória");
    n->kind = NODE_INT;
    n->as.int_value = v;
    return n;
}

static Node *node_binop(char op, Node *left, Node *right) {
    Node *n = (Node *)calloc(1, sizeof(Node));
    if (!n) die("Sem memória");
    n->kind = NODE_BINOP;
    n->as.binop.op = op;
    n->as.binop.left = left;
    n->as.binop.right = right;
    return n;
}

static void node_free(Node *n) {
    if (!n) return;
    if (n->kind == NODE_BINOP) {
        node_free(n->as.binop.left);
        node_free(n->as.binop.right);
    }
    free(n);
}

/* =========================
 *  Parser
 * ========================= */

typedef struct {
    const char *src;
    Lexer lx;
    Token cur;
} Parser;

static void parser_init(Parser *p, const char *src) {
    p->src = src;
    lexer_init(&p->lx, src);
    p->cur = lexer_next(&p->lx);
}

static void parser_error_at(Parser *p, size_t pos, const char *msg) {
    size_t start = pos;
    while (start > 0 && p->src[start - 1] != '\n') start--;
    size_t end = pos;
    while (p->src[end] && p->src[end] != '\n') end++;

    fprintf(stderr, "Erro de sintaxe: %s\n", msg);
    fprintf(stderr, "Perto de pos %zu:\n", pos);
    fprintf(stderr, "  %.*s\n", (int)(end - start), p->src + start);
    fprintf(stderr, "  %*s^\n", (int)(pos - start + 2), "");
    exit(1);
}

static void expect(Parser *p, TokenKind k, const char *what) {
    if (p->cur.kind != k) {
        char buf[160];
        snprintf(buf, sizeof(buf), "Esperava %s", what);
        parser_error_at(p, p->cur.pos, buf);
    }
}

static void advance(Parser *p) {
    p->cur = lexer_next(&p->lx);
}

static Node *parse_expr(Parser *p);

static Node *parse_expr(Parser *p) {
    if (p->cur.kind == TOK_INT) {
        long v = p->cur.int_value;
        advance(p);
        return node_int(v);
    }

    if (p->cur.kind == TOK_LPAREN) {
        advance(p); /* '(' */

        Node *left = parse_expr(p);

        expect(p, TOK_OP, "um operador (+, -, *, /)");
        char op = p->cur.op;
        advance(p); /* op */

        Node *right = parse_expr(p);

        expect(p, TOK_RPAREN, "')'");
        advance(p); /* ')' */

        return node_binop(op, left, right);
    }

    if (p->cur.kind == TOK_INVALID) {
        parser_error_at(p, p->cur.pos, "Caractere inválido");
    }

    parser_error_at(p, p->cur.pos, "Esperava inteiro ou '('");
    return NULL; /* unreachable */
}

static Node *parse_program(const char *src) {
    Parser p;
    parser_init(&p, src);

    Node *ast = parse_expr(&p);

    if (p.cur.kind != TOK_EOF) {
        parser_error_at(&p, p.cur.pos, "Sobrou texto depois do fim da expressão");
    }

    return ast;
}

/* =========================
 *  Emissão e geração de ASM
 * ========================= */

static void emit(FILE *out, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    fputc('\n', out);
    va_end(args);
}

/*
 * Gera código para deixar o valor da expressão em %rax.
 *
 * Estratégia:
 *   - gera direita -> %rax
 *   - push %rax
 *   - gera esquerda -> %rax
 *   - pop %rbx   (direita)
 *   - aplica operação (%rax = esquerda op direita)
 */
static void gen_expr(FILE *out, const Node *e) {
    if (!e) return;

    switch (e->kind) {
        case NODE_INT:
            emit(out, "  mov $%ld, %%rax", e->as.int_value);
            return;

        case NODE_BINOP: {
            const char op = e->as.binop.op;

            gen_expr(out, e->as.binop.right);
            emit(out, "  push %%rax");

            gen_expr(out, e->as.binop.left);
            emit(out, "  pop %%rbx");

            switch (op) {
                case '+':
                    emit(out, "  add %%rbx, %%rax");
                    break;
                case '-':
                    emit(out, "  sub %%rbx, %%rax"); /* rax = rax - rbx */
                    break;
                case '*':
                    emit(out, "  imul %%rbx, %%rax");
                    break;
                case '/':
                    /* numerador em rdx:rax; cqo faz extensão de sinal */
                    emit(out, "  cqo");
                    emit(out, "  idiv %%rbx");
                    break;
                default:
                    die("Operador inválido na AST: '%c'", op);
            }
            return;
        }
        default:
            die("Tipo de nó desconhecido");
    }
}

static void write_output_s(const char *out_path, const Node *ast) {
    FILE *out = fopen(out_path, "w");
    if (!out) die("Erro criando '%s': %s", out_path, strerror(errno));

    emit(out, ".section .text");
    emit(out, ".globl _start");
    emit(out, "_start:");

    gen_expr(out, ast);

    emit(out, "  call imprime_num");
    emit(out, "  call sair");
    emit(out, "  .include \"runtime.s\"");

    fclose(out);
}

/* =========================
 *  Main
 * ========================= */

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s arquivo.ci\n", argv[0]);
        return 1;
    }

    char *src = read_entire_file(argv[1]);
    Node *ast = parse_program(src);

    write_output_s("output.s", ast);

    node_free(ast);
    free(src);
    return 0;
}
