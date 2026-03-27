# Compilador Fun — Linguagem com Funções, Escopo Local e Recursão

Este projeto implementa um compilador para a linguagem **Fun** (Atividade 11), uma evolução da linguagem Cmd.  
O compilador lê um arquivo-fonte contendo declarações globais de variáveis e funções, constrói uma Árvore Sintática Abstrata (AST), realiza verificação semântica (aridade, escopo, shadowing) e gera código assembly x86-64 utilizando convenções de chamada (calling conventions) baseadas na pilha.

O programa gerado executa o bloco `main`, calcula o valor retornado e imprime o resultado utilizando o runtime fornecido.

---

## Integrantes

- **ÊMILLY EDUARDA CAROLINY SILVA** – 20220166942
- **LUIZ MANOEL BARBOSA RAMALHO** – 20220096614
- **REUBEN LISBOA RAMALHO CLAUDINO** – 20210024602
- **VICTOR PESSOA OLIVEIRA ORTINS** – 20210024667

---

## Estrutura do projeto

No diretório do projeto, espera-se uma organização semelhante a esta:

    .
    ├── fun_compiler.c
    ├── runtime.s
    ├── README.md
    ├── testes/
    │   ├── pos_fatorial.fun
    │   ├── pos_shadowing.fun
    │   ├── pos_mult_args.fun
    │   ├── neg_aridade.fun
    │   ├── neg_var_as_fun.fun
    │   └── neg_undeclared.fun
    └── src/
        ├── ast.c
        ├── ast.h
        ├── lexical.c
        ├── lexical.h
        ├── parser.c
        ├── parser.h
        ├── semantic.c
        ├── semantic.h
        ├── utils.c
        └── utils.h

### Função dos módulos principais

* `fun_compiler.c`: Ponto de entrada do compilador e responsável pela geração de código assembly gerindo o registro de ativação (stack frame) das funções.
* `src/ast.*`: Definição e construção da Árvore Sintática Abstrata, agora com suporte a nós de funções e chamadas.
* `src/lexical.*`: Analisador léxico adaptado para reconhecer novas palavras-chave e vírgulas.
* `src/parser.*`: Analisador sintático com *lookahead* para distinguir chamadas de funções de referências a variáveis.
* `src/semantic.*`: Análise semântica contendo tabelas de símbolos para escopo local e global.

---

## A Linguagem Fun

A gramática da linguagem Fun adiciona a capacidade de declarar funções e um bloco principal à linguagem base, sendo especificada da seguinte forma:

    <programa> ::= <decl>* 'main' '{' <cmd>* 'return' <exp>';' '}'
    <decl>     ::= <vardecl> | <fundecl>
    <fundecl>  ::= 'fun' <ident> '(' <arglist>? ')' '{' <vardecl>* <cmd>* 'return' <exp> ';' '}'
    <arglist>  ::= <ident> | <ident>','<arglist>
    <vardecl>  ::= 'var' <ident> '=' <exp> ';'
    <ident>    ::= <letra><letra_digito>*
    <cmd>      ::= <if> | <while> | <atrib>
    <if>       ::= 'if' <exp> '{' <cmd>* '}' 'else' '{' <cmd>* '}'
    <while>    ::= 'while' <exp> '{' <cmd>* '}'
    <atrib>    ::= <ident> '=' <exp> ';'
    <exp>      ::= <exp_a> (('<' | '>' | '==') <exp_a>)*
    <exp_a>    ::= <exp_m> (('+' | '-') <exp_m>)*
    <exp_m>    ::= <prim> (('*' | '/') <prim>)*
    <prim>     ::= <num> | <ident> | '(' <exp> ')' | <fun>
    <fun>      ::= <ident> '(' <params>? ')'
    <params>   ::= <exp> | <exp> ',' <params>
    <num>      ::= <digito><digito>*

---

## Funcionalidades implementadas

O compilador reflete as seguintes características exigidas para funções:

1. **Análise Léxica:** Reconhecimento do token de vírgula (`,`) para separar parâmetros e reconhecimento das novas palavras-chave `fun`, `var` e `main`.
2. **Análise Sintática:** Identificação de chamadas de função checando se o token após um identificador é um abre-parênteses `(`.
3. **Análise Semântica:** * Verificação de chamadas validando se a função existe e se o número de parâmetros reais corresponde aos parâmetros formais declarados.
   * Suporte a funções recursivas inserindo a função no escopo antes da verificação de seu próprio corpo.
   * Implementação de escopos locais usando a tabela de símbolos da função atual antes de buscar na tabela global.
4. **Geração de Código (Calling Conventions):**
   * Parâmetros são empilhados na ordem inversa antes da instrução `CALL`.
   * O corpo da função aloca espaço na pilha subtraindo do registrador `%rsp` e usa o `%rbp` como ponteiro base (*frame pointer*).
   * O valor de retorno da função é armazenado no registrador `%rax`.

---

## Como compilar e executar

### 1. Compilar o compilador
Utilize o `gcc` para compilar o código fonte em C:

    gcc -Wall -Wextra -std=c11 fun_compiler.c src/*.c -o fun_compiler

### 2. Executar o compilador
Passe o arquivo fonte da linguagem Fun (com extensão `.fun`) como argumento:

    ./fun_compiler testes/pos_fatorial.fun

Isso irá gerar um arquivo `output.s` contendo as instruções em assembly. O código gerado posiciona as funções na seção TEXT e o programa principal no rótulo `_start`.

### 3. Montar e Linkar o Assembly
Como o arquivo `output.s` inclui as rotinas básicas com a diretiva `.include "runtime.s"`, basta montar e linkar com o GNU Assembler e Linker:

    as -o output.o output.s
    ld -o programa output.o
    ./programa


---