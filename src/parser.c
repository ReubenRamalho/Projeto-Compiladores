#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "utils.h"

static BinOpKind token_to_binop(TokenKind kind) {
    switch (kind) {
        case TOK_OP_ADD: return OP_ADD;
        case TOK_OP_SUB: return OP_SUB;
        case TOK_OP_MUL: return OP_MUL;
        case TOK_OP_DIV: return OP_DIV;
        case TOK_OP_LT:  return OP_LT;
        case TOK_OP_GT:  return OP_GT;
        case TOK_OP_EQ:  return OP_EQ;
        default:
            die("Erro interno: token inválido para operador binário");
            return OP_ADD;
    }
}

void parser_init(Parser *p, const char *src) {
    p->src = src;
    lexer_init(&p->lx, src);
    p->cur = lexer_next(&p->lx);
}

void parser_error_at(Parser *p, size_t pos, const char *msg) {
    size_t start = pos;
    size_t end = pos;

    while (start > 0 && p->src[start - 1] != '\n') start--;
    while (p->src[end] && p->src[end] != '\n') end++;

    fprintf(stderr, "Erro de sintaxe: %s\n", msg);
    fprintf(stderr, "Perto de pos %zu:\n", pos);
    fprintf(stderr, "  %.*s\n", (int)(end - start), p->src + start);
    fprintf(stderr, "  %*s^\n", (int)(pos - start), "");
    exit(1);
}

void advance(Parser *p) {
    token_free(&p->cur);
    p->cur = lexer_next(&p->lx);
}

void expect(Parser *p, TokenKind kind, const char *what) {
    if (p->cur.kind != kind) {
        char buf[160];
        snprintf(buf, sizeof(buf), "Esperava %s", what);
        parser_error_at(p, p->cur.pos, buf);
    }
}

Expr *parse_prim(Parser *p) {
    if (p->cur.kind == TOK_INT) {
        long v = p->cur.int_value;
        advance(p);
        return expr_int(v);
    }

    if (p->cur.kind == TOK_IDENT) {
        Expr *e = expr_var(p->cur.lexeme);
        advance(p);
        return e;
    }

    if (p->cur.kind == TOK_LPAREN) {
        Expr *e;
        advance(p);
        e = parse_exp(p);
        expect(p, TOK_RPAREN, "')'");
        advance(p);
        return e;
    }

    if (p->cur.kind == TOK_INVALID) {
        parser_error_at(p, p->cur.pos, "Caractere inválido");
    }

    parser_error_at(p, p->cur.pos, "Esperava inteiro, identificador ou '('");
    return NULL;
}

Expr *parse_exp_m(Parser *p) {
    Expr *left = parse_prim(p);

    while (p->cur.kind == TOK_OP_MUL || p->cur.kind == TOK_OP_DIV) {
        TokenKind op = p->cur.kind;
        Expr *right;

        advance(p);
        right = parse_prim(p);
        left = expr_binop(token_to_binop(op), left, right);
    }

    return left;
}

Expr *parse_exp_a(Parser *p) {
    Expr *left = parse_exp_m(p);

    while (p->cur.kind == TOK_OP_ADD || p->cur.kind == TOK_OP_SUB) {
        TokenKind op = p->cur.kind;
        Expr *right;

        advance(p);
        right = parse_exp_m(p);
        left = expr_binop(token_to_binop(op), left, right);
    }

    return left;
}

Expr *parse_exp(Parser *p) {
    Expr *left = parse_exp_a(p);

    while (p->cur.kind == TOK_OP_LT || p->cur.kind == TOK_OP_GT || p->cur.kind == TOK_OP_EQ) {
        TokenKind op = p->cur.kind;
        Expr *right;

        advance(p);
        right = parse_exp_a(p);
        left = expr_binop(token_to_binop(op), left, right);
    }

    return left;
}

Decl *parse_decl(Parser *p) {
    char *name;
    Expr *value;
    Decl *decl;

    expect(p, TOK_IDENT, "identificador");
    name = p->cur.lexeme;
    p->cur.lexeme = NULL;
    advance(p);

    expect(p, TOK_EQUAL, "'='");
    advance(p);

    value = parse_exp(p);

    expect(p, TOK_SEMI, "';'");
    advance(p);

    decl = decl_new(name, value);
    free(name);
    return decl;
}

static Cmd *parse_assign_cmd(Parser *p) {
    char *name;
    Expr *value;
    Cmd *cmd;

    expect(p, TOK_IDENT, "identificador");
    name = p->cur.lexeme;
    p->cur.lexeme = NULL;
    advance(p);

    expect(p, TOK_EQUAL, "'='");
    advance(p);

    value = parse_exp(p);

    expect(p, TOK_SEMI, "';'");
    advance(p);

    cmd = cmd_assign(name, value);
    free(name);
    return cmd;
}

CmdList parse_cmd_block(Parser *p, TokenKind terminator) {
    CmdList list;
    cmd_list_init(&list);

    while (p->cur.kind != terminator) {
        if (p->cur.kind == TOK_EOF) {
            parser_error_at(p, p->cur.pos, "Fim inesperado do arquivo dentro de bloco");
        }
        cmd_list_add(&list, parse_cmd(p));
    }

    return list;
}

Cmd *parse_cmd(Parser *p) {
    if (p->cur.kind == TOK_IDENT) {
        return parse_assign_cmd(p);
    }

    if (p->cur.kind == TOK_IF) {
        Expr *condition;
        CmdList then_branch;
        CmdList else_branch;
        Cmd *cmd;

        advance(p);
        condition = parse_exp(p);

        expect(p, TOK_LBRACE, "'{'");
        advance(p);
        then_branch = parse_cmd_block(p, TOK_RBRACE);
        expect(p, TOK_RBRACE, "'}'");
        advance(p);

        expect(p, TOK_ELSE, "'else'");
        advance(p);

        expect(p, TOK_LBRACE, "'{'");
        advance(p);
        else_branch = parse_cmd_block(p, TOK_RBRACE);
        expect(p, TOK_RBRACE, "'}'");
        advance(p);

        cmd = cmd_if(condition, &then_branch, &else_branch);
        free(then_branch.items);
        free(else_branch.items);
        return cmd;
    }

    if (p->cur.kind == TOK_WHILE) {
        Expr *condition;
        CmdList body;
        Cmd *cmd;

        advance(p);
        condition = parse_exp(p);

        expect(p, TOK_LBRACE, "'{'");
        advance(p);
        body = parse_cmd_block(p, TOK_RBRACE);
        expect(p, TOK_RBRACE, "'}'");
        advance(p);

        cmd = cmd_while(condition, &body);
        free(body.items);
        return cmd;
    }

    parser_error_at(p, p->cur.pos, "Esperava um comando (if, while ou atribuição)");
    return NULL;
}

Program *parse_program(const char *src) {
    Parser p;
    Program *program;
    CmdList body;

    parser_init(&p, src);
    program = program_new();

    while (p.cur.kind == TOK_IDENT) {
        Decl *d = parse_decl(&p);
        program_add_decl(program, d);
    }

    expect(&p, TOK_LBRACE, "'{' para iniciar o corpo do programa");
    advance(&p);

    body = parse_cmd_block(&p, TOK_RETURN);
    program_set_body(program, &body);
    free(body.items);

    expect(&p, TOK_RETURN, "'return'");
    advance(&p);

    program_set_result(program, parse_exp(&p));

    expect(&p, TOK_SEMI, "';' após a expressão de retorno");
    advance(&p);

    expect(&p, TOK_RBRACE, "'}' para encerrar o programa");
    advance(&p);

    if (p.cur.kind != TOK_EOF) {
        parser_error_at(&p, p.cur.pos, "Sobrou texto depois do fim do programa");
    }

    token_free(&p.cur);
    return program;
}
