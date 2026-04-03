# Compilador Fun — Linguagem com Funções, Escopo Local, Recursão, Booleanos e Arrays

Este projeto implementa um compilador para a linguagem **Fun**, desenvolvida ao longo da disciplina de Compiladores e estendida no **Projeto Final**.  
A versão atual do compilador suporta:

- variáveis globais e locais;
- funções com parâmetros;
- chamadas de função, incluindo **recursão direta**;
- bloco principal `main`;
- comandos condicionais e de repetição;
- operadores aritméticos e relacionais;
- **valores booleanos** (`true` e `false`);
- **operadores lógicos** (`and`, `or` e `not`);
- **arrays de inteiros** (globais e locais, com acesso por índices dinâmicos).

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
│   ├── pos_bool.fun
│   └── pos_arrays.fun
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

- `fun_compiler.c`: ponto de entrada do compilador e responsável pela geração de código assembly, incluindo funções, chamadas, variáveis globais, variáveis locais na pilha e mapeamento de memória para arrays.
- `src/ast.*`: definição e construção da AST.
- `src/lexical.*`: analisador léxico.
- `src/parser.*`: analisador sintático.
- `src/semantic.*`: análise semântica com verificação de escopo, checagem de tipos de arrays e chamadas de função.
- `src/utils.*`: funções auxiliares de leitura de arquivo, mensagens de erro e emissão do assembly.
- `runtime.s`: rotinas auxiliares fornecidas para impressão e finalização do programa.

---

## A linguagem Fun

A linguagem Fun é uma evolução da linguagem Cmd, adicionando **funções**, **variáveis locais** e um **bloco principal**. No projeto final, foram adicionados também **booleanos**, **operadores lógicos** e **arrays de inteiros**. A proposta do projeto final permite implementar extensões simples como essas.

### Estrutura geral da linguagem

```text
<programa> ::= <decl>* 'main' '{' <cmd>* 'return' <exp> ';' '}'
<decl>     ::= <vardecl> | <fundecl>
<fundecl>  ::= 'fun' <ident> '(' <arglist>? ')' '{' <vardecl>* <cmd>* 'return' <exp> ';' '}'
<vardecl>  ::= 'var' <ident> '=' <exp> ';' | 'var' <ident> '[' <int> ']' ';'
<cmd>      ::= <if> | <while> | <atrib>
<if>       ::= 'if' <exp> '{' <cmd>* '}' 'else' '{' <cmd>* '}'
<while>    ::= 'while' <exp> '{' <cmd>* '}'
<atrib>    ::= <ident> '=' <exp> ';' | <ident> '[' <exp> ']' '=' <exp> ';'
```

### Expressões suportadas

A linguagem aceita:

- inteiros positivos;
- variáveis simples e acessos a posições de arrays (`arr[i]`);
- chamadas de função;
- operadores aritméticos: `+`, `-`, `*`, `/`;
- operadores relacionais: `<`, `>`, `==`, `!=`, `<=`, `>=`;
- valores booleanos: `true`, `false`;
- operadores lógicos:
  - `not` (negação),
  - `and` (conjunção),
  - `or` (disjunção).

---

## Extensões implementadas no Projeto Final

No Projeto Final, o grupo adicionou três grandes extensões à linguagem Fun:

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

Esses operadores permitem escrever expressões booleanas diretamente na linguagem, tornando os testes em `if`, `while` e `return` mais expressivos. As avaliações geram instruções nativas x86-64 (`and`, `or`), normalizando os operandos para garantir que valores diferentes de zero se tornem estritamente `1`.

### 3. Arrays de Inteiros
A linguagem agora suporta declaração, escrita e leitura em arrays de tamanho fixo:

- **Declaração:** `var arr[10];` (Aloca memória no `.bss` ou na pilha local calculando o deslocamento necessário).
- **Escrita:** `arr[0] = 50;` ou `arr[i * 2] = 100;`
- **Leitura:** `x = arr[2];`

A geração de código para arrays traduz o acesso para o poderoso modo de endereçamento indexado escalado do x86-64: `(base, indice, 8)`.

---

## Funcionalidades implementadas

### 1. Análise léxica
O compilador reconhece:

- identificadores e inteiros;
- palavras-chave:
  - `fun`, `var`, `main`, `if`, `else`, `while`, `return`, `true`, `false`, `and`, `or`, `not`
- símbolos:
  - `(` `)` `{` `}` `[` `]` `,` `;`
- operadores:
  - `=`
  - `+` `-` `*` `/`
  - `<` `>` `==` `!=` `<=` `>=`

### 2. Análise sintática
O parser realiza:

- reconhecimento de declarações globais de variáveis e arrays;
- reconhecimento de declarações de função;
- leitura da lista de parâmetros formais;
- distinção via *lookahead* entre atribuição escalar (`x = 1`) e indexada (`x[0] = 1`);
- parsing do bloco `main`;
- parsing de expressões aritméticas, relacionais e lógicas com rigorosa precedência (ex: `not` > `and` > `or`).

### 3. Análise semântica
A análise semântica verifica:

- uso de variável antes da declaração e redeclaração no mesmo escopo;
- **shadowing** de globais por parâmetros e variáveis locais;
- integridade no uso de arrays (ex: impede operações diretas com o nome do array sem índice, ou uso de índices em variáveis escalares);
- se uma função chamada foi declarada e se a aridade corresponde;
- se uma variável está sendo usada como função e vice-versa.

### 4. Geração de código
O compilador gera assembly x86-64 com:

- variáveis e arrays globais na seção `.bss` (calculando tamanho `N * 8` bytes);
- funções definidas com rótulos e variáveis locais em *stack frames* estendidos para comportar arrays locais;
- chamadas com `call`, retorno em `%rax` e parâmetros passados pela pilha.

---

## Convenções de chamada adotadas

A implementação das funções segue convenções simples baseadas na pilha:

- os argumentos da chamada são empilhados antes do `call`;
- a função cria seu próprio *stack frame* usando `%rbp`;
- variáveis locais, arrays locais e parâmetros são acessados por offsets relativos a `%rbp`;
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
./fun_compiler testes/pos_arrays.fun
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

## Exemplo completo com as 3 extensões

### Arquivo `pos_arrays.fun`

```fun
var memoria[5];

fun preenche() {
    var i = 0;
    while i < 5 {
        memoria[i] = i * 10;
        i = i + 1;
    }
    return true;
}

main {
    var pronto = preenche();
    var resultado = 0;
    
    if pronto and (memoria[2] == 20) {
        resultado = memoria[4];
    } else {
        resultado = 0;
    }
    
    return resultado;
}
```

### Compilação e execução

```bash
./fun_compiler testes/pos_arrays.fun
as -o output.o output.s
ld -o programa output.o
./programa
```

### Saída esperada

```text
40
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

### Erro no uso de Arrays (Indexando Escalar)

```fun
var x = 10;

main {
    x[0] = 5; 
    // Erro semântico: a variável 'x' não é um array e não pode ser indexada na atribuição
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

Este projeto implementa um compilador para a linguagem Fun estendida com:

- variáveis globais e locais;
- arrays de inteiros locais e globais;
- funções com parâmetros e recursão;
- escopo local e shadowing;
- comandos `if` e `while`;
- operadores aritméticos e relacionais;
- valores booleanos e operadores lógicos (`and`, `or`, `not`);
- geração nativa de código assembly x86-64.

Ao final, o compilador gera um programa executável capaz de avaliar o bloco principal `main` e imprimir o valor retornado.
