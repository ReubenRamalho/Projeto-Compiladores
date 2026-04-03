#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        case TOK_OP_LE:  return OP_LE;
        case TOK_OP_GE:  return OP_GE;
        case TOK_OP_NE:  return OP_NE;
        case TOK_AND:    return OP_AND;
        case TOK_OR:     return OP_OR;
        default:
            die("Erro interno: token inválido para operador binário");
            return OP_ADD;
    }
}

static char *xstrdup(const char *s) {
    size_t n;
    char *copy;
    if (!s) return NULL;
    n = strlen(s);
    copy = (char *)malloc(n + 1);
    if (!copy) die("Sem memória");
    memcpy(copy, s, n + 1);
    return copy;
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

static TokenKind peek_kind(Parser *p) {
    Lexer temp_lx = p->lx;
    Token next = lexer_next(&temp_lx);
    TokenKind kind = next.kind;
    token_free(&next);
    return kind;
}

Expr *parse_prim(Parser *p) {
    if (p->cur.kind == TOK_INT) {
        long v = p->cur.int_value;
        advance(p);
        return expr_int(v);
    }

    if (p->cur.kind == TOK_TRUE) {
        advance(p);
        return expr_bool(1);
    }

    if (p->cur.kind == TOK_FALSE) {
        advance(p);
        return expr_bool(0);
    }

    if (p->cur.kind == TOK_IDENT) {
        char *name = p->cur.lexeme;
        p->cur.lexeme = NULL;
        
        TokenKind next_k = peek_kind(p);
        
        if (next_k == TOK_LPAREN) {
            ExprList args;
            Expr *call_expr;
            expr_list_init(&args);
            
            advance(p);
            advance(p);
            
            if (p->cur.kind != TOK_RPAREN) {
                while (1) {
                    expr_list_add(&args, parse_exp(p));
                    if (p->cur.kind == TOK_COMMA) {
                        advance(p);
                    } else {
                        break;
                    }
                }
            }
            expect(p, TOK_RPAREN, "')' fechando os argumentos da função");
            advance(p);
            call_expr = expr_call(name, &args);
            free(name);
            free(args.items);
            return call_expr;
            
        } else if (next_k == TOK_LBRACKET) { 
            Expr *arr_expr;
            advance(p); 
            advance(p);
            
            Expr *index = parse_exp(p);
            
            expect(p, TOK_RBRACKET, "']' fechando o índice do array");
            advance(p);
            
            arr_expr = expr_array_access(name, index);
            free(name);
            return arr_expr;
            
        } else {
            Expr *var_expr;
            advance(p);
            var_expr = expr_var(name);
            free(name);
            return var_expr;
        }
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

    parser_error_at(p, p->cur.pos, "Esperava inteiro, booleano, identificador ou '('");
    return NULL;
}

Expr *parse_exp_u(Parser *p) {
    if (p->cur.kind == TOK_NOT) {
        Expr *operand;
        advance(p);
        operand = parse_exp_u(p);
        return expr_unop(UOP_NOT, operand);
    }
    return parse_prim(p);
}

Expr *parse_exp_m(Parser *p) {
    Expr *left = parse_exp_u(p);
    while (p->cur.kind == TOK_OP_MUL || p->cur.kind == TOK_OP_DIV) {
        TokenKind op = p->cur.kind;
        Expr *right;
        advance(p);
        right = parse_exp_u(p);
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

Expr *parse_exp_cmp(Parser *p) {
    Expr *left = parse_exp_a(p);
    while (p->cur.kind == TOK_OP_LT || p->cur.kind == TOK_OP_GT || p->cur.kind == TOK_OP_EQ ||
           p->cur.kind == TOK_OP_LE || p->cur.kind == TOK_OP_GE || p->cur.kind == TOK_OP_NE) {
        TokenKind op = p->cur.kind;
        Expr *right;
        advance(p);
        right = parse_exp_a(p);
        left = expr_binop(token_to_binop(op), left, right);
    }
    return left;
}

Expr *parse_exp_and(Parser *p) {
    Expr *left = parse_exp_cmp(p);
    while (p->cur.kind == TOK_AND) {
        Expr *right;
        advance(p);
        right = parse_exp_cmp(p);
        left = expr_binop(OP_AND, left, right);
    }
    return left;
}

Expr *parse_exp_or(Parser *p) {
    Expr *left = parse_exp_and(p);
    while (p->cur.kind == TOK_OR) {
        Expr *right;
        advance(p);
        right = parse_exp_and(p);
        left = expr_binop(OP_OR, left, right);
    }
    return left;
}

Expr *parse_exp(Parser *p) {
    return parse_exp_or(p);
}

VarDecl *parse_vardecl(Parser *p) {
    char *name;
    VarDecl *vd;
    expect(p, TOK_VAR, "'var'");
    advance(p);
    expect(p, TOK_IDENT, "identificador da variável");
    name = p->cur.lexeme;
    p->cur.lexeme = NULL;
    advance(p);

    if (p->cur.kind == TOK_LBRACKET) { 
        size_t size;
        advance(p);
        
        expect(p, TOK_INT, "tamanho inteiro do array");
        size = (size_t)p->cur.int_value;
        advance(p);
        
        expect(p, TOK_RBRACKET, "']'");
        advance(p);
        
        expect(p, TOK_SEMI, "';'");
        advance(p);
        
        vd = var_decl_array_new(name, size);
        free(name);
        return vd;
    }

    expect(p, TOK_EQUAL, "'='");
    advance(p);

    Expr *value = parse_exp(p);

    expect(p, TOK_SEMI, "';'");
    advance(p);
    vd = var_decl_new(name, value);
    free(name);
    return vd;
}

Decl *parse_fundecl(Parser *p) {
    char *name;
    StringList params;
    VarDeclList locals;
    CmdList body;
    Expr *result_expr;
    Decl *fun_decl;

    expect(p, TOK_FUN, "'fun'");
    advance(p);
    expect(p, TOK_IDENT, "nome da função");
    name = p->cur.lexeme;
    p->cur.lexeme = NULL;
    advance(p);
    expect(p, TOK_LPAREN, "'('");
    advance(p);

    string_list_init(&params);
    if (p->cur.kind != TOK_RPAREN) {
        while (1) {
            expect(p, TOK_IDENT, "parâmetro formal");
            string_list_add(&params, xstrdup(p->cur.lexeme));
            advance(p);
            if (p->cur.kind == TOK_COMMA) advance(p);
            else break;
        }
    }
    expect(p, TOK_RPAREN, "')'");
    advance(p);
    expect(p, TOK_LBRACE, "'{' para abrir o corpo da função");
    advance(p);

    var_decl_list_init(&locals);
    while (p->cur.kind == TOK_VAR) {
        var_decl_list_add(&locals, parse_vardecl(p));
    }

    body = parse_cmd_block(p, TOK_RETURN);
    expect(p, TOK_RETURN, "'return' no final da função");
    advance(p);
    result_expr = parse_exp(p);
    expect(p, TOK_SEMI, "';' após a expressão de retorno");
    advance(p);
    expect(p, TOK_RBRACE, "'}' fechando o corpo da função");
    advance(p);

    fun_decl = decl_fun_new(name, &params, &locals, &body, result_expr);
    free(name);
    free(params.items);
    free(locals.items);
    free(body.items);
    return fun_decl;
}

Decl *parse_decl(Parser *p) {
    if (p->cur.kind == TOK_VAR) {
        VarDecl *vd = parse_vardecl(p);
        Decl *d = decl_var_new(vd->name, vd->value);
        
        d->as.var_decl.is_array = vd->is_array;
        d->as.var_decl.array_size = vd->array_size;

        free(vd->name);
        free(vd);
        return d;
    }
    if (p->cur.kind == TOK_FUN) {
        return parse_fundecl(p);
    }
    parser_error_at(p, p->cur.pos, "Esperava declaração de 'var' ou 'fun'");
    return NULL;
}

static Cmd *parse_assign_cmd(Parser *p) {
    char *name;
    Cmd *cmd;
    expect(p, TOK_IDENT, "identificador");
    name = p->cur.lexeme;
    p->cur.lexeme = NULL;
    advance(p);

    if (p->cur.kind == TOK_LBRACKET) { 
        advance(p);
        Expr *index = parse_exp(p);
        expect(p, TOK_RBRACKET, "']'");
        advance(p);
        
        expect(p, TOK_EQUAL, "'='");
        advance(p);
        
        Expr *value = parse_exp(p);
        
        expect(p, TOK_SEMI, "';'");
        advance(p);
        
        cmd = cmd_array_assign(name, index, value);
        free(name);
        return cmd;
    }

    expect(p, TOK_EQUAL, "'='");
    advance(p);

    Expr *value = parse_exp(p);

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
    CmdList main_body;
    Expr *main_result;
    parser_init(&p, src);
    program = program_new();

    while (p.cur.kind == TOK_VAR || p.cur.kind == TOK_FUN) {
        program_add_decl(program, parse_decl(&p));
    }

    expect(&p, TOK_MAIN, "'main' para iniciar o bloco principal"); 
    advance(&p);
    expect(&p, TOK_LBRACE, "'{' para iniciar o corpo do main");
    advance(&p);
    main_body = parse_cmd_block(&p, TOK_RETURN);
    expect(&p, TOK_RETURN, "'return'");
    advance(&p);
    main_result = parse_exp(&p);
    expect(&p, TOK_SEMI, "';' após a expressão de retorno");
    advance(&p);
    expect(&p, TOK_RBRACE, "'}' para encerrar o programa");
    advance(&p);
    if (p.cur.kind != TOK_EOF) {
        parser_error_at(&p, p.cur.pos, "Sobrou texto depois do fim do programa");
    }
    program_set_main(program, &main_body, main_result);
    free(main_body.items);
    token_free(&p.cur);
    return program;
}
