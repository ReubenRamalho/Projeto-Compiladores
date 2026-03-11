/*
 * EV Compiler (Expressões com Variáveis)
 *
 * Lê um arquivo-fonte da linguagem EV, constrói a AST do programa,
 * realiza análise semântica para verificar uso de variáveis e gera
 * um arquivo output.s em assembly x86-64 GNU as.
 *
 * Gramática base:
 *   <programa> ::= <decl>* <result>
 *   <decl>     ::= <ident> '=' <exp> ';'
 *   <result>   ::= '=' <exp>
 *   <exp>      ::= <exp_m> (('+' | '-') <exp_m>)*
 *   <exp_m>    ::= <prim> (('*' | '/') <prim>)*
 *   <prim>     ::= <num> | <ident> | '(' <exp> ')'
 *
 * Estratégia de geração:
 *   - Cada expressão deixa seu resultado em %rax
 *   - Variáveis são alocadas em .bss com .lcomm <nome>, 8
 *   - Declarações geram código da expressão e depois:
 *       mov %rax, <variavel>
 *   - Referências a variáveis geram:
 *       mov <variavel>, %rax
 *
 * Saída (output.s):
 *   .section .bss
 *      <variaveis>
 *   .section .text
 *   .globl _start
 *   _start:
 *      <codigo das declaracoes>
 *      <codigo da expressao final>
 *      call imprime_num
 *      call sair
 *      .include "runtime.s"
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/ast.h"
#include "src/parser.h"
#include "src/semantic.h"
#include "src/utils.h"

/*
 * Gera código para uma expressão, deixando o valor final em %rax.
 *
 * Estratégia para binários:
 *   1. gera lado direito em %rax
 *   2. push %rax
 *   3. gera lado esquerdo em %rax
 *   4. pop %rbx   ; recupera o lado direito
 *   5. aplica a operação
 */
static void gen_expr(FILE *out, const Expr *e) {
    if (!e) return;

    switch (e->kind) {
        case EXPR_INT:
            emit(out, "  mov $%ld, %%rax", e->as.int_value);
            return;

        case EXPR_VAR:
            emit(out, "  mov %s, %%rax", e->as.var_name);
            return;

        case EXPR_BINOP: {
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
                    emit(out, "  sub %%rbx, %%rax");
                    break;
                case '*':
                    emit(out, "  imul %%rbx, %%rax");
                    break;
                case '/':
                    emit(out, "  cqo");
                    emit(out, "  idiv %%rbx");
                    break;
                default:
                    die("Operador inválido na AST: '%c'", op);
            }
            return;
        }
        default:
            die("Tipo de expressão desconhecido");
    }
}

/* Emite as variáveis na seção BSS. */
static void gen_bss(FILE *out, const Program *program) {
    size_t i;

    emit(out, ".section .bss");
    for (i = 0; i < program->decl_count; i++) {
        emit(out, "  .lcomm %s, 8", program->decls[i]->name);
    }
    emit(out, "");
}

/* Emite o código das declarações e da expressão final. */
static void gen_text(FILE *out, const Program *program) {
    size_t i;

    emit(out, ".section .text");
    emit(out, ".globl _start");
    emit(out, "_start:");

    for (i = 0; i < program->decl_count; i++) {
        const Decl *d = program->decls[i];

        emit(out, "  # %s = ...", d->name);
        gen_expr(out, d->value);
        emit(out, "  mov %%rax, %s", d->name);
    }

    emit(out, "  # expressao final");
    gen_expr(out, program->result_expr);

    emit(out, "  call imprime_num");
    emit(out, "  call sair");
    emit(out, "  .include \"runtime.s\"");
}

static void write_output_s(const char *out_path, const Program *program) {
    FILE *out = fopen(out_path, "w");
    if (!out) die("Erro criando '%s': %s", out_path, strerror(errno));

    gen_bss(out, program);
    gen_text(out, program);

    fclose(out);
}

int main(int argc, char **argv) {
    char *src;
    Program *program;

    if (argc != 2) {
        fprintf(stderr, "Uso: %s arquivo.ev\n", argv[0]);
        return 1;
    }

    src = read_entire_file(argv[1]);
    program = parse_program(src);

    semantic_check_program(program);
    write_output_s("output.s", program);

    program_free(program);
    free(src);
    return 0;
}