/*
 * EC1 Interpreter (Expressões Constantes 1) - Atividade 05
 *
 * Lê um arquivo contendo uma expressão EC1, constrói uma AST,
 * imprime a árvore sintática e avalia o resultado diretamente (interpretação).
 */

#include <stdio.h>
#include <stdlib.h>

#include "src/ast.h"
#include "src/lexical.h"
#include "src/parser.h"
#include "src/utils.h"


static void print_expr(const Node *e) {
    if (!e) return;

    if (e->kind == NODE_INT) { // Se for um valor, só imprime
        printf("%ld", e->as.int_value);
    } else if (e->kind == NODE_BINOP) {
        // Se for uma operação binária, envolve de parênteses
        printf("(");
        print_expr(e->as.binop.left);
        printf("%c", e->as.binop.op);
        print_expr(e->as.binop.right);
        printf(")");
    }
}


static long eval_expr(const Node *e) {
    if (!e) return 0;

    if (e->kind == NODE_INT) { // Se é constante, só pega o valor
        return e->as.int_value;
    } 
    
    if (e->kind == NODE_BINOP) {
        // Chama recursivamente pra pegar o valor da esquerda e o da direita
        long left_val = eval_expr(e->as.binop.left);
        long right_val = eval_expr(e->as.binop.right);
        
        // Identifica o operador e faz o cálculo
        switch (e->as.binop.op) {
            case '+': return left_val + right_val;
            case '-': return left_val - right_val;
            case '*': return left_val * right_val;
            case '/': 
                if (right_val == 0) die("Erro: Divisão por zero na interpretação!");
                return left_val / right_val;
            default:
                die("Operador inválido na AST: '%c'", e->as.binop.op);
        }
    }
    
    return 0;
}

/* =========================
 * Main
 * ========================= */

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s arquivo.ci\n", argv[0]);
        return 1;
    }

    /* 1. Análise Léxica e Sintática (gera a AST) */
    char *src = read_entire_file(argv[1]);
    Node *ast = parse_program(src);

    /* 2. Impressão da Árvore (opcional, mas recomendado na Atividade 05) */
    printf("Árvore Sintática lida: ");
    print_expr(ast);
    printf("\n");

    /* 3. Interpretação (obtém o valor final do programa) */
    long result = eval_expr(ast);
    printf("Resultado interpretado: %ld\n", result);

    /* Limpeza */
    node_free(ast);
    free(src);
    return 0;
}