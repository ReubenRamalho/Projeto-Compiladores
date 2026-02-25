/*
 * EC2 Compiler (Expressões Constantes 2)
 *
 * Lê um arquivo .ci contendo uma expressão EC2, constrói uma AST respeitando
 * a precedência e associatividade dos operadores, e gera um output.s
 * que calcula a expressão e imprime usando imprime_num/sair do runtime.s.
 *
 * Gramática EC2:
 * <programa> ::= <exp_a>
 * <exp_a>    ::= <exp_m> (('+' | '-') <exp_m>)*
 * <exp_m>    ::= <prim>  (('*' | '/') <prim>)*
 * <prim>     ::= <num>   | '(' <exp_a> ')'
 * <num>      ::= <digito>+
 *
 * Geração de código:
 * - Resultado final SEMPRE em %rax
 * - Estratégia com pilha (push/pop) recomendada para evitar “falta de registradores”
 * e preservar a ordem correta em '-' e '/'.
 *
 * Saída (output.s):
 * .section .text
 * .globl _start
 * _start:
 * <codigo da expr>
 * call imprime_num
 * call sair
 * .include "runtime.s"
 *
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/ast.h"
#include "src/lexical.h"
#include "src/parser.h"
#include "src/utils.h"

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
