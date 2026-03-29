/*
 * Fun Compiler
 *
 * Compilador para a linguagem Fun da Atividade 11.
 *
 * A linguagem aceita:
 * - declarações globais de variáveis (var) e funções (fun);
 * - chamadas de função recursivas;
 * - passagem de múltiplos argumentos;
 * - escopos locais (shadowing de globais);
 * - bloco principal (main).
 *
 * O compilador constrói uma AST, realiza análise semântica verificando aridade
 * e escopo, e gera um arquivo output.s em assembly x86-64 GNU as seguindo
 * convenções de chamada baseadas na pilha.
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
 * @brief Estrutura auxiliar para mapear variáveis locais aos seus offsets no %rbp.
 */
typedef struct {
    char *name;
    int offset;
} EnvVar;

/**
 * @brief Ambiente local usado durante a geração de código de uma função.
 */
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
    env->vars[env->count].name = (char *)name; // O nome pertence à AST, não damos free aqui
    env->vars[env->count].offset = offset;
    env->count++;
}

static int env_find(const LocalEnv *env, const char *name) {
    if (!env) return -1;
    for (size_t i = 0; i < env->count; i++) {
        if (strcmp(env->vars[i].name, name) == 0) {
            return env->vars[i].offset;
        }
    }
    return -1;
}

static void env_free(LocalEnv *env) {
    if (env->vars) free(env->vars);
}

/**
 * @brief Gera um rótulo numérico único para ifs e whiles.
 */
static long next_label_id(void) {
    static long counter = 0;
    return counter++;
}

static void gen_cmd_list(FILE *out, const CmdList *list, LocalEnv *env);

/**
 * @brief Gera código para uma expressão, deixando o resultado em %rax.
 * Agora suporta busca no ambiente local e chamadas de função (CALL).
 */
static void gen_expr(FILE *out, const Expr *e, LocalEnv *env) {
    int offset;
    long i;

    if (!e) return;

    switch (e->kind) {
        case EXPR_INT:
            emit(out, "  mov $%ld, %%rax", e->as.int_value);
            return;

        case EXPR_VAR:
            offset = env_find(env, e->as.var_name); // Primeiro procura local
            if (offset != -1) {
                // É local/parâmetro, usa endereçamento indireto via RBP
                emit(out, "  mov %d(%%rbp), %%rax", offset);
            } else {
                // É global, usa o rótulo
                emit(out, "  mov %s, %%rax", e->as.var_name);
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
            return;

        case EXPR_CALL:
            // 1. Avalia e empilha argumentos na ordem inversa (do último para o primeiro)
            for (i = (long)e->as.call.args.count - 1; i >= 0; i--) {
                gen_expr(out, e->as.call.args.items[i], env);
                emit(out, "  push %%rax");
            }
            // 2. Faz a chamada (vai empilhar o Endereço de Retorno automaticamente)
            emit(out, "  call %s", e->as.call.fun_name);
            // 3. Remove os argumentos da pilha somando ao RBP (limpeza do caller)
            if (e->as.call.args.count > 0) {
                emit(out, "  add $%ld, %%rsp", e->as.call.args.count * 8);
            }
            return;
    }
}

/**
 * @brief Gera código para um comando (if, while, assign), ciente do escopo local.
 */
static void gen_cmd(FILE *out, const Cmd *cmd, LocalEnv *env) {
    long id;
    int offset;

    switch (cmd->kind) {
        case CMD_ASSIGN:
            gen_expr(out, cmd->as.assign.value, env);
            offset = env_find(env, cmd->as.assign.name);
            if (offset != -1) {
                // Salva na pilha local
                emit(out, "  mov %%rax, %d(%%rbp)", offset);
            } else {
                // Salva na memória global
                emit(out, "  mov %%rax, %s", cmd->as.assign.name);
            }
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

/**
 * @brief Gera código para uma lista de comandos, repassando o ambiente.
 */
static void gen_cmd_list(FILE *out, const CmdList *list, LocalEnv *env) {
    size_t i;
    for (i = 0; i < list->count; i++) {
        gen_cmd(out, list->items[i], env);
    }
}

/**
 * @brief Gera o código completo de uma função, gerindo seu Stack Frame (Registro de Ativação).
 */
static void gen_func(FILE *out, const Decl *decl) {
    LocalEnv env;
    size_t i;
    long local_bytes = decl->as.fun_decl.locals.count * 8;

    env_init(&env);

    // Mapeia parâmetros. Devido aos 'push' em ordem inversa e ao prólogo:
    // RBP+16 (RBP Antigo), RBP+24 (ER), RBP+32 (1º Arg), RBP+40 (2º Arg)...
    for (i = 0; i < decl->as.fun_decl.params.count; i++) {
            env_add(&env, decl->as.fun_decl.params.items[i], local_bytes + 16 + (i * 8));
    }

    
    // Mapeia variáveis locais. Crescem a partir do RBP (RBP+0, RBP+8...).
    for (i = 0; i < decl->as.fun_decl.locals.count; i++) {
        env_add(&env, decl->as.fun_decl.locals.items[i]->name, i * 8);
    }

    // Assinatura e Prólogo da Função
    emit(out, "");
    emit(out, "%s:", decl->as.fun_decl.name);
    emit(out, "  push %%rbp");
    if (local_bytes > 0) {
        emit(out, "  sub $%ld, %%rsp", local_bytes);
    }
    emit(out, "  mov %%rsp, %%rbp");

    // Inicialização das variáveis locais
    for (i = 0; i < decl->as.fun_decl.locals.count; i++) {
        gen_expr(out, decl->as.fun_decl.locals.items[i]->value, &env);
        emit(out, "  mov %%rax, %d(%%rbp)", (int)(i * 8));
    }

    // Corpo e Expressão de Retorno
    gen_cmd_list(out, &decl->as.fun_decl.body, &env);
    gen_expr(out, decl->as.fun_decl.result_expr, &env);

    // Epílogo da Função
    if (local_bytes > 0) {
        emit(out, "  add $%ld, %%rsp", local_bytes);
    }
    emit(out, "  pop %%rbp");
    emit(out, "  ret");

    env_free(&env);
}

/**
 * @brief Emite apenas variáveis globais (ignora funções) na seção .bss.
 */
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

/**
 * @brief Emite a inicialização global, bloco main e, por fim, funções no final do arquivo.
 */
static void gen_text(FILE *out, const Program *program) {
    size_t i;

    emit(out, ".section .text");
    emit(out, ".globl _start");
    emit(out, "_start:");

    // Inicializa as globais antes de entrar no main
    for (i = 0; i < program->decl_count; i++) {
        if (program->decls[i]->kind == DECL_VAR) {
            gen_expr(out, program->decls[i]->as.var_decl.value, NULL);
            emit(out, "  mov %%rax, %s", program->decls[i]->as.var_decl.name);
        }
    }

    // Executa os comandos e o retorno do bloco main
    gen_cmd_list(out, &program->main_body, NULL);
    gen_expr(out, program->main_result, NULL);
    
    emit(out, "  call imprime_num");
    emit(out, "  call sair");
    emit(out, "");

    // Gera o código para todas as funções declaradas
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

    gen_bss(out, program); // Pega todas as variáveis e coloca no .bss
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