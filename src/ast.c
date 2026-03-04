#include <stdlib.h>

#include "ast.h"
#include "utils.h"

Node *node_int(long v) {
    Node *n = (Node *)calloc(1, sizeof(Node));
    if (!n) die("Sem memória");
    n->kind = NODE_INT;
    n->as.int_value = v;
    return n;
}

Node *node_binop(char op, Node *left, Node *right) {
    Node *n = (Node *)calloc(1, sizeof(Node));
    if (!n) die("Sem memória");
    n->kind = NODE_BINOP;
    n->as.binop.op = op;
    n->as.binop.left = left;
    n->as.binop.right = right;
    return n;
}

void node_free(Node *n) {
    if (!n) return;
    if (n->kind == NODE_BINOP) {
        node_free(n->as.binop.left);
        node_free(n->as.binop.right);
    }
    free(n);
}