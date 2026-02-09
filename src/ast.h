#pragma once

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

Node *node_int(long v);

Node *node_binop(char op, Node *left, Node *right);

void node_free(Node *n);