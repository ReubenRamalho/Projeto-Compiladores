/*
 * Fun Compiler
 *
 * Compilador para a linguagem Fun estendida para o Projeto Final.
 *
 * Extensões adicionadas:
 * - valores booleanos: true e false;
 * - operadores lógicos: and, or e not.
 *
 * Convenção adotada:
 * - valores booleanos são representados como 0 (falso) e 1 (verdadeiro);
 * - expressões lógicas sempre normalizam o resultado para 0 ou 1 em %rax.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/ast.h"
#include "src/parser.h"
#include "src/semantic.h"
#include "src/utils.h"

typedef struct {
    char *name;
    int offset;
} EnvVar;

typedef struct {
    EnvVar *vars;
    size_t count;
    size_t capacity;
} LocalEnv;

static void env_init(LocalEnv *env) {
    env->vars = NULL;
    env->count = 0;
    env->capacity = 0;
}

static void env_add(LocalEnv *env, const char *name, int offset) {
    if (env->count == env->capacity) {
        env->capacity = (env->capacity == 0) ? 8 : env->capacity * 2;
        env->vars = (EnvVar *)realloc(env->vars, env->capacity * sizeof(EnvVar));
        if (!env->vars) die("Sem memória no LocalEnv");
    }
    env->vars[env->count].name = (char *)name;
    env->vars[env->count].offset = offset;
    env->count++;
}

static int env_find(const LocalEnv *env, const char *name) {
    size_t i;
    if (!env) return -1;
    for (i = 0; i < env->count; i++) {
        if (strcmp(env->vars[i].name, name) == 0) {
            return env->vars[i].offset;
        }
    }
    return -1;
}

static void env_free(LocalEnv *env) {
    free(env->vars);
}

static long next_label_id(void) {
    static long counter = 0;
    return counter++;
}

static void gen_cmd_list(FILE *out, const CmdList *list, LocalEnv *env);

static void normalize_rax_to_bool(FILE *out) {
    emit(out, "  cmp $0, %%rax");
    emit(out, "  mov $0, %%rax");
    emit(out, "  setne %%al");
}

static void gen_expr(FILE *out, const Expr *e, LocalEnv *env) {
    int offset;
    long i;

    if (!e) return;

    switch (e->kind) {
        case EXPR_INT:
            emit(out, "  mov $%ld, %%rax", e->as.int_value);
            return;

        case EXPR_BOOL:
            emit(out, "  mov $%d, %%rax", e->as.bool_value ? 1 : 0);
            return;

        case EXPR_VAR:
            offset = env_find(env, e->as.var_name);
            if (offset != -1) {
                emit(out, "  mov %d(%%rbp), %%rax", offset);
            } else {
                emit(out, "  mov %s, %%rax", e->as.var_name);
            }
            return;

        case EXPR_UNOP:
            gen_expr(out, e->as.unop.operand, env);
            switch (e->as.unop.op) {
                case UOP_NOT:
                    emit(out, "  cmp $0, %%rax");
                    emit(out, "  mov $0, %%rax");
                    emit(out, "  sete %%al");
                    return;
            }
            return;

        case EXPR_BINOP:
            gen_expr(out, e->as.binop.right, env);
            emit(out, "  push %%rax");
            gen_expr(out, e->as.binop.left, env);
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
                case OP_LE:
                case OP_GE:
                case OP_NE:
                    emit(out, "  xor %%rcx, %%rcx");
                    emit(out, "  cmp %%rbx, %%rax");
                    if (e->as.binop.op == OP_LT) emit(out, "  setl %%cl");
                    else if (e->as.binop.op == OP_GT) emit(out, "  setg %%cl");
                    else if (e->as.binop.op == OP_EQ) emit(out, "  sete %%cl");
                    else if (e->as.binop.op == OP_LE) emit(out, "  setle %%cl");
                    else if (e->as.binop.op == OP_GE) emit(out, "  setge %%cl");
                    else emit(out, "  setne %%cl");
                    emit(out, "  mov %%rcx, %%rax");
                    return;
                case OP_AND:
                    normalize_rax_to_bool(out);
                    emit(out, "  mov %%rax, %%rcx");
                    emit(out, "  mov %%rbx, %%rax");
                    normalize_rax_to_bool(out);
                    emit(out, "  and %%rcx, %%rax");
                    return;
                case OP_OR:
                    normalize_rax_to_bool(out);
                    emit(out, "  mov %%rax, %%rcx");
                    emit(out, "  mov %%rbx, %%rax");
                    normalize_rax_to_bool(out);
                    emit(out, "  or %%rcx, %%rax");
                    normalize_rax_to_bool(out);
                    return;
            }
            return;

        case EXPR_CALL:
            for (i = (long)e->as.call.args.count - 1; i >= 0; i--) {
                gen_expr(out, e->as.call.args.items[i], env);
                emit(out, "  push %%rax");
            }
            emit(out, "  call %s", e->as.call.fun_name);
            if (e->as.call.args.count > 0) {
                emit(out, "  add $%ld, %%rsp", e->as.call.args.count * 8);
            }
            return;
    }
}

static void gen_cmd(FILE *out, const Cmd *cmd, LocalEnv *env) {
    long id;
    int offset;

    switch (cmd->kind) {
        case CMD_ASSIGN:
            gen_expr(out, cmd->as.assign.value, env);
            offset = env_find(env, cmd->as.assign.name);
            if (offset != -1) emit(out, "  mov %%rax, %d(%%rbp)", offset);
            else emit(out, "  mov %%rax, %s", cmd->as.assign.name);
            return;

        case CMD_IF:
            id = next_label_id();
            gen_expr(out, cmd->as.if_cmd.condition, env);
            emit(out, "  cmp $0, %%rax");
            emit(out, "  jz Lfalso%ld", id);
            gen_cmd_list(out, &cmd->as.if_cmd.then_branch, env);
            emit(out, "  jmp Lfim%ld", id);
            emit(out, "Lfalso%ld:", id);
            gen_cmd_list(out, &cmd->as.if_cmd.else_branch, env);
            emit(out, "Lfim%ld:", id);
            return;

        case CMD_WHILE:
            id = next_label_id();
            emit(out, "Linicio%ld:", id);
            gen_expr(out, cmd->as.while_cmd.condition, env);
            emit(out, "  cmp $0, %%rax");
            emit(out, "  jz Lfim%ld", id);
            gen_cmd_list(out, &cmd->as.while_cmd.body, env);
            emit(out, "  jmp Linicio%ld", id);
            emit(out, "Lfim%ld:", id);
            return;
    }
}

static void gen_cmd_list(FILE *out, const CmdList *list, LocalEnv *env) {
    size_t i;
    for (i = 0; i < list->count; i++) {
        gen_cmd(out, list->items[i], env);
    }
}

static void gen_func(FILE *out, const Decl *decl) {
    LocalEnv env;
    size_t i;
    long local_bytes = decl->as.fun_decl.locals.count * 8;

    env_init(&env);
    for (i = 0; i < decl->as.fun_decl.params.count; i++) {
        env_add(&env, decl->as.fun_decl.params.items[i], (int)(local_bytes + 16 + (i * 8)));
    }
    for (i = 0; i < decl->as.fun_decl.locals.count; i++) {
        env_add(&env, decl->as.fun_decl.locals.items[i]->name, (int)(i * 8));
    }

    emit(out, "");
    emit(out, "%s:", decl->as.fun_decl.name);
    emit(out, "  push %%rbp");
    if (local_bytes > 0) emit(out, "  sub $%ld, %%rsp", local_bytes);
    emit(out, "  mov %%rsp, %%rbp");

    for (i = 0; i < decl->as.fun_decl.locals.count; i++) {
        gen_expr(out, decl->as.fun_decl.locals.items[i]->value, &env);
        emit(out, "  mov %%rax, %d(%%rbp)", (int)(i * 8));
    }

    gen_cmd_list(out, &decl->as.fun_decl.body, &env);
    gen_expr(out, decl->as.fun_decl.result_expr, &env);

    if (local_bytes > 0) emit(out, "  add $%ld, %%rsp", local_bytes);
    emit(out, "  pop %%rbp");
    emit(out, "  ret");
    env_free(&env);
}

static void gen_bss(FILE *out, const Program *program) {
    size_t i;
    emit(out, ".section .bss");
    for (i = 0; i < program->decl_count; i++) {
        if (program->decls[i]->kind == DECL_VAR) {
            emit(out, "  .lcomm %s, 8", program->decls[i]->as.var_decl.name);
        }
    }
    emit(out, "");
}

static void gen_text(FILE *out, const Program *program) {
    size_t i;
    emit(out, ".section .text");
    emit(out, ".globl _start");
    emit(out, "_start:");

    for (i = 0; i < program->decl_count; i++) {
        if (program->decls[i]->kind == DECL_VAR) {
            gen_expr(out, program->decls[i]->as.var_decl.value, NULL);
            emit(out, "  mov %%rax, %s", program->decls[i]->as.var_decl.name);
        }
    }

    gen_cmd_list(out, &program->main_body, NULL);
    gen_expr(out, program->main_result, NULL);
    emit(out, "  call imprime_num");
    emit(out, "  call sair");
    emit(out, "");

    for (i = 0; i < program->decl_count; i++) {
        if (program->decls[i]->kind == DECL_FUN) {
            gen_func(out, program->decls[i]);
        }
    }

    emit(out, "");
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
        fprintf(stderr, "Uso: %s arquivo.fun\n", argv[0]);
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
