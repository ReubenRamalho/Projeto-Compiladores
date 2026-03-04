#pragma once

/**
 * @brief Enumeração dos tipos de nós da AST.
 */
typedef enum { NODE_INT, NODE_BINOP } NodeKind;

/**
 * @brief Estrutura que representa um nó da AST.
 */
typedef struct Node {
    NodeKind kind; ///< Tipo do nó.
    union {
        long int_value;
        struct {
            char op;                
            struct Node *left;      
            struct Node *right;    
        } binop; 
    } as;
} Node;

/**
 * @brief Cria um nó inteiro na AST.
 * @param v Valor inteiro.
 * @return Ponteiro para o nó criado.
 */
Node *node_int(long v);

/**
 * @brief Cria um nó de operação binária na AST.
 * @param op Operador.
 * @param left Ponteiro para o nó filho esquerdo.
 * @param right Ponteiro para o nó filho direito.
 * @return Ponteiro para o nó criado.
 */
Node *node_binop(char op, Node *left, Node *right);

/**
 * @brief Libera a memória de um nó da AST (recursivamente).
 * @param n Ponteiro para o nó a ser liberado.
 */
void node_free(Node *n);