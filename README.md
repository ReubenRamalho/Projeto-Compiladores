# Compilador Fun — Linguagem com Funções, Escopo Local, Recursão e Booleanos

Este projeto implementa um compilador para a linguagem **Fun**, desenvolvida ao longo da disciplina de Compiladores e estendida no **Projeto Final**.  
A versão atual do compilador suporta:

- variáveis globais e locais;
- funções com parâmetros;
- chamadas de função, incluindo **recursão direta**;
- bloco principal `main`;
- comandos condicionais e de repetição;
- operadores aritméticos e relacionais;
- **valores booleanos** (`true` e `false`);
- **operadores lógicos** (`and`, `or` e `not`).

O compilador lê um arquivo-fonte da linguagem Fun, constrói uma **Árvore Sintática Abstrata (AST)**, realiza **análise semântica** e gera código **assembly x86-64**, utilizando convenções de chamada baseadas na pilha. O programa gerado executa o bloco `main`, calcula o valor retornado e o imprime utilizando o `runtime.s`.

---

## Integrantes

- **ÊMILLY EDUARDA CAROLINY SILVA** – 20220166942
- **LUIZ MANOEL BARBOSA RAMALHO** – 20220096614
- **REUBEN LISBOA RAMALHO CLAUDINO** – 20210024602
- **VICTOR PESSOA OLIVEIRA ORTINS** – 20210024667

---

## Estrutura do projeto

A organização esperada do projeto é semelhante a esta:

```text
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
│   ├── neg_undeclared.fun
│   └── pos_bool.fun
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
```

### Função dos módulos principais

- `fun_compiler.c`: ponto de entrada do compilador e responsável pela geração de código assembly, incluindo funções, chamadas, variáveis globais e variáveis locais na pilha.
- `src/ast.*`: definição e construção da AST.
- `src/lexical.*`: analisador léxico.
- `src/parser.*`: analisador sintático.
- `src/semantic.*`: análise semântica com verificação de escopo e chamadas de função.
- `src/utils.*`: funções auxiliares de leitura de arquivo, mensagens de erro e emissão do assembly.
- `runtime.s`: rotinas auxiliares fornecidas para impressão e finalização do programa.

---

## A linguagem Fun

A linguagem Fun é uma evolução da linguagem Cmd, adicionando **funções**, **variáveis locais** e um **bloco principal**. No projeto final, foram adicionados também **booleanos** e **operadores lógicos**. A proposta do projeto final permite implementar extensões simples como essas.

### Estrutura geral da linguagem

```text
<programa> ::= <decl>* 'main' '{' <cmd>* 'return' <exp> ';' '}'
<decl>     ::= <vardecl> | <fundecl>
<fundecl>  ::= 'fun' <ident> '(' <arglist>? ')' '{' <vardecl>* <cmd>* 'return' <exp> ';' '}'
<vardecl>  ::= 'var' <ident> '=' <exp> ';'
<cmd>      ::= <if> | <while> | <atrib>
<if>       ::= 'if' <exp> '{' <cmd>* '}' 'else' '{' <cmd>* '}'
<while>    ::= 'while' <exp> '{' <cmd>* '}'
<atrib>    ::= <ident> '=' <exp> ';'
```

### Expressões suportadas

A linguagem aceita:

- inteiros positivos;
- variáveis;
- chamadas de função;
- operadores aritméticos: `+`, `-`, `*`, `/`;
- operadores relacionais: `<`, `>`, `==`, `!=`, `<=`, `>=`;
- valores booleanos: `true`, `false`;
- operadores lógicos:
  - `not` (negação),
  - `and` (conjunção),
  - `or` (disjunção).

### Exemplo de programa

```fun
var x = 10;
var y = false;

fun menor(a, b) {
    return a < b;
}

fun teste(n) {
    var ok = true;
    if not (n < 0) {
        ok = ok and true;
    } else {
        ok = false;
    }
    return ok;
}

main {
    y = teste(x) or menor(3, 2);
    return y;
}
```

---

## Extensões implementadas no Projeto Final

No Projeto Final, o grupo adicionou duas extensões simples à linguagem Fun:

### 1. Valores booleanos
Foram adicionados os literais:

- `true`
- `false`

Internamente, os valores booleanos são representados no assembly como:

- `true` → `1`
- `false` → `0`

### 2. Operadores lógicos
Foram adicionados os operadores:

- `and`
- `or`
- `not`

Esses operadores permitem escrever expressões booleanas diretamente na linguagem, tornando os testes em `if`, `while` e `return` mais expressivos.

Exemplos:

```fun
return true;
return false;
return not false;
return true and false;
return (x < 10) or (y == 0);
```

---

## Funcionalidades implementadas

### 1. Análise léxica
O compilador reconhece:

- identificadores;
- inteiros;
- palavras-chave:
  - `fun`
  - `var`
  - `main`
  - `if`
  - `else`
  - `while`
  - `return`
  - `true`
  - `false`
  - `and`
  - `or`
  - `not`
- símbolos:
  - `(` `)` `{` `}` `,` `;`
- operadores:
  - `=`
  - `+` `-` `*` `/`
  - `<` `>` `==` `!=` `<=` `>=`

### 2. Análise sintática
O parser realiza:

- reconhecimento de declarações globais de variáveis;
- reconhecimento de declarações de função;
- leitura da lista de parâmetros formais;
- distinção entre variável e chamada de função usando *lookahead*;
- parsing do bloco `main`;
- parsing de expressões aritméticas, relacionais e lógicas com precedência.

### 3. Análise semântica
A análise semântica verifica:

- uso de variável antes da declaração;
- redeclaração no mesmo escopo;
- uso correto de escopo global e local;
- **shadowing** de globais por parâmetros e variáveis locais;
- se uma função chamada foi declarada;
- se a quantidade de argumentos reais corresponde à aridade da função;
- se uma variável está sendo usada como função;
- se uma função está sendo usada como variável.

### 4. Geração de código
O compilador gera assembly x86-64 com:

- variáveis globais na seção `.bss`;
- funções definidas com rótulos;
- chamadas com `call`;
- retorno com `ret`;
- parâmetros passados pela pilha;
- variáveis locais armazenadas no *stack frame* da função;
- valor de retorno sempre em `%rax`.

Para os booleanos:

- `true` gera `1` em `%rax`;
- `false` gera `0` em `%rax`;
- operadores lógicos também retornam `0` ou `1`.

---

## Convenções de chamada adotadas

A implementação das funções segue convenções simples baseadas na pilha:

- os argumentos da chamada são empilhados antes do `call`;
- a função cria seu próprio *stack frame* usando `%rbp`;
- variáveis locais e parâmetros são acessados por offsets relativos a `%rbp`;
- o valor de retorno é colocado em `%rax`;
- o chamador é responsável por limpar os argumentos da pilha após a chamada.

---

## Como compilar

Para compilar o compilador, use:

```bash
gcc -Wall -Wextra -std=c11 fun_compiler.c src/*.c -o fun_compiler
```

### Sobre as flags
- `-Wall -Wextra`: habilitam avisos úteis do compilador.
- `-std=c11`: compila o projeto seguindo o padrão C11.

---

## Como executar

Depois de compilar o compilador, execute:

```bash
./fun_compiler testes/pos_fatorial.fun
```

Isso irá gerar o arquivo:

```text
output.s
```

---

## Como montar e linkar o assembly gerado

Como o arquivo `output.s` já inclui o runtime com:

```asm
.include "runtime.s"
```

basta executar:

```bash
as -o output.o output.s
ld -o programa output.o
```

---

## Como executar o programa final

Após montar e linkar, execute:

```bash
./programa
```

O valor retornado pelo bloco `main` será impresso no terminal.

---

## Exemplo completo

### Arquivo `pos_bool.fun`

```fun
var x = true;
var y = false;

fun check(a, b) {
    var r = false;
    if a and not b {
        r = true;
    } else {
        r = false;
    }
    return r;
}

main {
    y = check(x, false) or false;
    return y;
}
```

### Compilação e execução

```bash
./fun_compiler testes/pos_bool.fun
as -o output.o output.s
ld -o programa output.o
./programa
```

### Saída esperada

```text
1
```

---

## Exemplos de erros semânticos

### Variável não declarada

```fun
main {
    x = 10;
    return x;
}
```

### Chamada com número errado de argumentos

```fun
fun soma(a, b) {
    return a + b;
}

main {
    return soma(10);
}
```

### Variável usada como função

```fun
var x = 10;

main {
    return x();
}
```

---

## Observações importantes

- O compilador gera código para arquitetura **x86-64**.
- O valor final de uma expressão é sempre produzido em `%rax`.
- Condições em `if` e `while` usam a convenção de que:
  - `0` significa falso
  - qualquer valor diferente de zero significa verdadeiro
- Os booleanos literais `true` e `false` são normalizados para `1` e `0`.
- O projeto não faz otimizações; o foco é a correção do processo de compilação.
- O warning abaixo, caso apareça, indica apenas uma função auxiliar não utilizada:
  ```text
  warning: '...' defined but not used [-Wunused-function]
  ```
  Isso não impede a compilação, mas pode indicar código que pode ser removido.

---

## Ambiente de desenvolvimento

- Sistema operacional: Linux x86-64
- Linguagem de implementação do compilador: C
- Compilador C: `gcc`
- Montador e linker: `as` e `ld` (GNU binutils)

---

## Extensão dos arquivos de entrada

Os programas da linguagem Fun usam a extensão:

```text
.fun
```

Exemplo:

```text
programa.fun
```

---

## Resumo

Este projeto implementa um compilador para a linguagem Fun com:

- variáveis globais e locais;
- funções com parâmetros;
- recursão;
- escopo local;
- chamadas de função;
- comandos `if` e `while`;
- operadores aritméticos e relacionais;
- valores booleanos;
- operadores lógicos `and`, `or` e `not`;
- geração de código assembly x86-64.

Ao final, o compilador gera um programa executável capaz de avaliar o bloco principal `main` e imprimir o valor retornado.
