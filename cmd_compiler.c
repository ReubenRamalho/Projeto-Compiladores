/*
 * Cmd Compiler
 *
 * Compilador para a linguagem Cmd da Atividade 10.
 *
 * A linguagem aceita:
 * - declarações iniciais de variáveis;
 * - comandos de atribuição, if/else e while;
 * - expressão final introduzida por return.
 *
 * O compilador constrói uma AST, realiza análise semântica e gera um
 * arquivo output.s em assembly x86-64 GNU as.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/ast.h"
#include "src/parser.h"
#include "src/semantic.h"
#include "src/utils.h"

/**
 * @brief Gera um rótulo numérico único para ifs e whiles.
 */
static long next_label_id(void) {
    static long counter = 0;
    return counter++;
}

/**
 * @brief Gera código para uma expressão, deixando o resultado em %rax.
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

        case EXPR_BINOP:
            gen_expr(out, e->as.binop.right);
            emit(out, "  push %%rax");
            gen_expr(out, e->as.binop.left);
            emit(out, "  pop %%rbx");

            switch (e->as.binop.op) {
                case OP_ADD:
                    emit(out, "  add %%rbx, %%rax");
                    return;
                case OP_SUB:
                    emit(out, "  sub %%rbx, %%rax");
                    return;
                case OP_MUL:
                    emit(out, "  imul %%rbx, %%rax");
                    return;
                case OP_DIV:
                    emit(out, "  cqo");
                    emit(out, "  idiv %%rbx");
                    return;
                case OP_LT:
                case OP_GT:
                case OP_EQ:
                    emit(out, "  xor %%rcx, %%rcx");
                    emit(out, "  cmp %%rbx, %%rax");
                    if (e->as.binop.op == OP_LT) {
                        emit(out, "  setl %%cl");
                    } else if (e->as.binop.op == OP_GT) {
                        emit(out, "  setg %%cl");
                    } else {
                        emit(out, "  setz %%cl");
                    }
                    emit(out, "  mov %%rcx, %%rax");
                    return;
            }
    }
}

static void gen_cmd_list(FILE *out, const CmdList *list);

/**
 * @brief Gera código para um comando.
 */
static void gen_cmd(FILE *out, const Cmd *cmd) {
    long id;

    switch (cmd->kind) {
        case CMD_ASSIGN:
            gen_expr(out, cmd->as.assign.value);
            emit(out, "  mov %%rax, %s", cmd->as.assign.name);
            return;

        case CMD_IF:
            id = next_label_id();
            gen_expr(out, cmd->as.if_cmd.condition);
            emit(out, "  cmp $0, %%rax");
            emit(out, "  jz Lfalso%ld", id);
            gen_cmd_list(out, &cmd->as.if_cmd.then_branch);
            emit(out, "  jmp Lfim%ld", id);
            emit(out, "Lfalso%ld:", id);
            gen_cmd_list(out, &cmd->as.if_cmd.else_branch);
            emit(out, "Lfim%ld:", id);
            return;

        case CMD_WHILE:
            id = next_label_id();
            emit(out, "Linicio%ld:", id);
            gen_expr(out, cmd->as.while_cmd.condition);
            emit(out, "  cmp $0, %%rax");
            emit(out, "  jz Lfim%ld", id);
            gen_cmd_list(out, &cmd->as.while_cmd.body);
            emit(out, "  jmp Linicio%ld", id);
            emit(out, "Lfim%ld:", id);
            return;
    }
}

/**
 * @brief Gera código para uma lista de comandos.
 */
static void gen_cmd_list(FILE *out, const CmdList *list) {
    size_t i;
    for (i = 0; i < list->count; i++) {
        gen_cmd(out, list->items[i]);
    }
}

/**
 * @brief Emite as variáveis globais na seção .bss.
 */
static void gen_bss(FILE *out, const Program *program) {
    size_t i;

    emit(out, ".section .bss");
    for (i = 0; i < program->decl_count; i++) {
        emit(out, "  .lcomm %s, 8", program->decls[i]->name);
    }
    emit(out, "");
}

/**
 * @brief Emite a seção .text com inicializações, comandos e retorno.
 */
static void gen_text(FILE *out, const Program *program) {
    size_t i;

    emit(out, ".section .text");
    emit(out, ".globl _start");
    emit(out, "_start:");

    for (i = 0; i < program->decl_count; i++) {
        gen_expr(out, program->decls[i]->value);
        emit(out, "  mov %%rax, %s", program->decls[i]->name);
    }

    gen_cmd_list(out, &program->body);
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
        fprintf(stderr, "Uso: %s arquivo.cmd\n", argv[0]);
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
