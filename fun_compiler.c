/*
 * Fun Plus Compiler
 *
 * Compilador para a linguagem Fun, agora com suporte nativo
 * a Arrays de Inteiros (declaração, leitura e escrita)!
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
 * Associação entre o nome de uma variável local/parâmetro e
 * o deslocamento (offset) dela em relação ao frame pointer.
 */
typedef struct {
    char *name;
    int offset;
} EnvVar;

/*
 * Ambiente local usado durante a geração de código.
 * Ele guarda todos os nomes acessíveis no escopo da função
 * e o offset correspondente de cada um na pilha.
 */
typedef struct {
    EnvVar *vars;
    size_t count;
    size_t capacity;
} LocalEnv;

/* Inicializa a estrutura de ambiente local vazia. */
static void env_init(LocalEnv *env) {
    env->vars = NULL;
    env->count = 0;
    env->capacity = 0;
}

/*
 * Adiciona uma nova entrada ao ambiente local.
 * Se necessário, aumenta o vetor dinamicamente.
 */
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

/*
 * Procura uma variável no ambiente local.
 * Retorna o offset associado ou -1 caso ela não exista.
 */
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

/* Libera o vetor interno do ambiente local. */
static void env_free(LocalEnv *env) {
    free(env->vars);
}

/*
 * Gera identificadores únicos para labels do assembly.
 * Isso evita colisão entre ifs, whiles e outros blocos.
 */
static long next_label_id(void) {
    static long counter = 0;
    return counter++;
}

/* Declaração antecipada, pois gen_cmd usa gen_cmd_list e vice-versa. */
static void gen_cmd_list(FILE *out, const CmdList *list, LocalEnv *env);

/*
 * Normaliza o valor de %rax para booleano:
 * qualquer valor diferente de zero vira 1, e zero continua 0.
 */
static void normalize_rax_to_bool(FILE *out) {
    emit(out, "  cmp $0, %%rax");
    emit(out, "  mov $0, %%rax");
    emit(out, "  setne %%al");
}

/*
 * Gera o código assembly de uma expressão.
 * O resultado final da expressão fica em %rax.
 */
static void gen_expr(FILE *out, const Expr *e, LocalEnv *env) {
    int offset;
    long i;

    if (!e) return;

    switch (e->kind) {
        case EXPR_INT:
            /* Carrega um literal inteiro diretamente em %rax. */
            emit(out, "  mov $%ld, %%rax", e->as.int_value);
            return;

        case EXPR_BOOL:
            /* Representa booleanos como 0 (false) ou 1 (true). */
            emit(out, "  mov $%d, %%rax", e->as.bool_value ? 1 : 0);
            return;

        case EXPR_VAR:
            /*
             * Primeiro tenta encontrar a variável no ambiente local.
             * Se não achar, assume que é uma variável global.
             */
            offset = env_find(env, e->as.var_name); 
            if (offset != -1) {
                emit(out, "  mov %d(%%rbp), %%rax", offset);
            } else {
                emit(out, "  mov %s, %%rax", e->as.var_name);
            }
            return;

        case EXPR_ARRAY_ACCESS: // NOVO: Geração de leitura de Array
            // 1. Avalia o índice e salva em %rcx
            gen_expr(out, e->as.array_access.index, env);
            emit(out, "  mov %%rax, %%rcx");
            
            // 2. Pega o endereço base do array e joga em %rbx
            offset = env_find(env, e->as.array_access.array_name);
            if (offset != -1) {
                // Array Local
                emit(out, "  lea %d(%%rbp), %%rbx", offset);
            } else {
                // Array Global (RIP-relative)
                emit(out, "  lea %s(%%rip), %%rbx", e->as.array_access.array_name);
            }
            
            // 3. Lê o valor na memória (endereço = base + índice * 8)
            emit(out, "  mov (%%rbx,%%rcx,8), %%rax");
            return;

        case EXPR_UNOP:
            /* Gera primeiro o operando e depois aplica o operador unário. */
            gen_expr(out, e->as.unop.operand, env);
            switch (e->as.unop.op) {
                case UOP_NOT:
                    /* Implementa negação lógica: !x */
                    emit(out, "  cmp $0, %%rax");
                    emit(out, "  mov $0, %%rax");
                    emit(out, "  sete %%al");
                    return;
            }
            return;

        case EXPR_BINOP:
            /*
             * Avalia a expressão da direita, empilha o resultado,
             * depois avalia a da esquerda. Ao final:
             *   - esquerda fica em %rax
             *   - direita fica em %rbx
             */
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
                    /*
                     * Para divisão inteira em x86-64:
                     * - cqo estende o sinal de %rax para %rdx:%rax
                     * - idiv divide por %rbx
                     */
                    emit(out, "  cqo");
                    emit(out, "  idiv %%rbx");
                    return;
                case OP_LT:
                case OP_GT:
                case OP_EQ:
                case OP_LE:
                case OP_GE:
                case OP_NE:
                    /*
                     * Operadores relacionais produzem 0 ou 1.
                     * O resultado é montado em %rcx e copiado para %rax.
                     */
                    emit(out, "  xor %%rcx, %%rcx");
                    emit(out, "  cmp %%rbx, %%rax");
                    if (e->as.binop.op == OP_LT) emit(out, "  setl %%cl");
                    else if (e->as.binop.op == OP_GT) emit(out, "  setg %%cl");
                    else if (e->as.binop.op == OP_EQ) emit(out, "  sete %%cl");
                    else if (e->as.binop.op == OP_LE) emit(out, "  setle %%cl");
                    else if (e->as.binop.op == OP_GE) emit(out, "  setge %%cl");
                    else if (e->as.binop.op == OP_NE) emit(out, "  setne %%cl");
                    else emit(out, "  setne %%cl");
                    emit(out, "  mov %%rcx, %%rax");
                    return;
                case OP_AND:
                    /*
                     * Normaliza os dois lados para booleano e aplica AND bit a bit.
                     */
                    normalize_rax_to_bool(out);
                    emit(out, "  mov %%rax, %%rcx");
                    emit(out, "  mov %%rbx, %%rax");
                    normalize_rax_to_bool(out);
                    emit(out, "  and %%rcx, %%rax");
                    return;
                case OP_OR:
                    /*
                     * Normaliza os dois lados para booleano, aplica OR
                     * e normaliza novamente o resultado final.
                     */
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
            /*
             * Empilha os argumentos da direita para a esquerda,
             * faz a chamada e depois limpa a pilha.
             */
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

/**
 * @brief Gera código para um comando (if, while, assign, array_assign).
 */
static void gen_cmd(FILE *out, const Cmd *cmd, LocalEnv *env) {
    long id;
    int offset;

    switch (cmd->kind) {
        case CMD_ASSIGN:
            /* Gera a expressão do lado direito e armazena no destino. */
            gen_expr(out, cmd->as.assign.value, env);
            offset = env_find(env, cmd->as.assign.name);
            if (offset != -1) {
                emit(out, "  mov %%rax, %d(%%rbp)", offset);
            } else {
                emit(out, "  mov %%rax, %s", cmd->as.assign.name);
            }
            return;

        case CMD_ARRAY_ASSIGN: // NOVO: Geração de Escrita em Array
            // 1. Avalia o valor e empilha
            gen_expr(out, cmd->as.array_assign.value, env);
            emit(out, "  push %%rax");
            
            // 2. Avalia o índice e move para %rcx
            gen_expr(out, cmd->as.array_assign.index, env);
            emit(out, "  mov %%rax, %%rcx");
            
            // 3. Restaura o valor para %rax
            emit(out, "  pop %%rax");
            
            // 4. Pega o endereço base do array
            offset = env_find(env, cmd->as.array_assign.name);
            if (offset != -1) {
                emit(out, "  lea %d(%%rbp), %%rbx", offset);
            } else {
                emit(out, "  lea %s(%%rip), %%rbx", cmd->as.array_assign.name);
            }
            
            // 5. Salva na memória
            emit(out, "  mov %%rax, (%%rbx,%%rcx,8)");
            return;

        case CMD_IF:
            /*
             * Estrutura de if/else:
             * - se condição for 0, pula para o bloco falso
             * - senão executa o bloco verdadeiro
             */
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
            /*
             * Estrutura de repetição while:
             * testa a condição, executa o corpo enquanto for verdadeira
             * e volta ao início do laço.
             */
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

/* Gera uma sequência de comandos na ordem em que aparecem na AST. */
static void gen_cmd_list(FILE *out, const CmdList *list, LocalEnv *env) {
    size_t i;
    for (i = 0; i < list->count; i++) {
        gen_cmd(out, list->items[i], env);
    }
}

/*
 * Gera o código assembly de uma função declarada no programa.
 * Aqui são montados:
 * - o ambiente local
 * - a área de variáveis locais na pilha
 * - o corpo da função
 * - a expressão de retorno
 */
static void gen_func(FILE *out, const Decl *decl) {
    LocalEnv env;
    size_t i;
    long local_bytes = 0;
    long current_offset = 0;

    env_init(&env);

    // ALTERADO: Calcula espaço necessário para variáveis escalares E arrays locais
    for (i = 0; i < decl->as.fun_decl.locals.count; i++) {
        if (decl->as.fun_decl.locals.items[i]->is_array) {
            local_bytes += decl->as.fun_decl.locals.items[i]->array_size * 8;
        } else {
            local_bytes += 8;
        }
    }

    // Mapeia parâmetros (Mantido, independente do tamanho do local_bytes)
    for (i = 0; i < decl->as.fun_decl.params.count; i++) {
        env_add(&env, decl->as.fun_decl.params.items[i], (int)(local_bytes + 16 + (i * 8)));
    }

    // ALTERADO: Mapeia variáveis locais e arrays dinamicamente
    for (i = 0; i < decl->as.fun_decl.locals.count; i++) {
        env_add(&env, decl->as.fun_decl.locals.items[i]->name, current_offset);
        if (decl->as.fun_decl.locals.items[i]->is_array) {
            current_offset += decl->as.fun_decl.locals.items[i]->array_size * 8;
        } else {
            current_offset += 8;
        }
    }

    /*
     * Prólogo da função:
     * - salva o base pointer antigo
     * - reserva espaço para as variáveis locais
     * - ajusta %rbp para o frame atual
     */
    emit(out, "");
    emit(out, "%s:", decl->as.fun_decl.name);
    emit(out, "  push %%rbp");
    if (local_bytes > 0) emit(out, "  sub $%ld, %%rsp", local_bytes);
    emit(out, "  mov %%rsp, %%rbp");

    // Inicialização apenas das variáveis escalares (Arrays locais iniciam com lixo de memória por padrão)
    for (i = 0; i < decl->as.fun_decl.locals.count; i++) {
        if (!decl->as.fun_decl.locals.items[i]->is_array) {
            gen_expr(out, decl->as.fun_decl.locals.items[i]->value, &env);
            emit(out, "  mov %%rax, %d(%%rbp)", env_find(&env, decl->as.fun_decl.locals.items[i]->name));
        }
    }

    /* Corpo da função e expressão final de retorno. */
    gen_cmd_list(out, &decl->as.fun_decl.body, &env);
    gen_expr(out, decl->as.fun_decl.result_expr, &env);

    /* Epílogo da função: desfaz a pilha e retorna. */
    if (local_bytes > 0) emit(out, "  add $%ld, %%rsp", local_bytes);
    emit(out, "  pop %%rbp");
    emit(out, "  ret");
    env_free(&env);
}

/*
 * Gera a seção .bss para variáveis globais não inicializadas.
 * Variáveis escalares ocupam 8 bytes e arrays ocupam N * 8 bytes.
 */
static void gen_bss(FILE *out, const Program *program) {
    size_t i;
    emit(out, ".section .bss");
    for (i = 0; i < program->decl_count; i++) {
        if (program->decls[i]->kind == DECL_VAR) {
            // ALTERADO: Aloca baseado no tamanho do array
            if (program->decls[i]->as.var_decl.is_array) {
                emit(out, "  .lcomm %s, %ld", program->decls[i]->as.var_decl.name, program->decls[i]->as.var_decl.array_size * 8);
            } else {
                emit(out, "  .lcomm %s, 8", program->decls[i]->as.var_decl.name);
            }
        }
    }
    emit(out, "");
}

/**
 * @brief Emite a inicialização global, bloco main e, por fim, funções.
 */
static void gen_text(FILE *out, const Program *program) {
    size_t i;
    emit(out, ".section .text");
    emit(out, ".globl _start");
    emit(out, "_start:");

    // Inicializa as globais antes de entrar no main (Apenas as escalares)
    for (i = 0; i < program->decl_count; i++) {
        if (program->decls[i]->kind == DECL_VAR && !program->decls[i]->as.var_decl.is_array) {
            gen_expr(out, program->decls[i]->as.var_decl.value, NULL);
            emit(out, "  mov %%rax, %s", program->decls[i]->as.var_decl.name);
        }
    }

    /* Executa o corpo principal do programa e imprime o resultado final. */
    gen_cmd_list(out, &program->main_body, NULL);
    gen_expr(out, program->main_result, NULL);
    emit(out, "  call imprime_num");
    emit(out, "  call sair");
    emit(out, "");

    /* Depois do _start, emite o código de todas as funções declaradas. */
    for (i = 0; i < program->decl_count; i++) {
        if (program->decls[i]->kind == DECL_FUN) {
            gen_func(out, program->decls[i]);
        }
    }

    emit(out, "");
    emit(out, "  .include \"runtime.s\"");
}

/*
 * Cria o arquivo de saída assembly e escreve nele as seções
 * geradas a partir do programa analisado.
 */
static void write_output_s(const char *out_path, const Program *program) {
    FILE *out = fopen(out_path, "w");
    if (!out) die("Erro criando '%s': %s", out_path, strerror(errno));

    gen_bss(out, program); 
    gen_text(out, program);
    fclose(out);
}

/*
 * Fluxo principal do compilador:
 * 1. valida argumentos
 * 2. lê o arquivo fonte inteiro
 * 3. faz parsing
 * 4. executa checagem semântica
 * 5. gera o assembly em output.s
 * 6. libera a memória usada
 */
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
