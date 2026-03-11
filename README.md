# Compilador EV — Expressões com Variáveis

Este projeto implementa um compilador simples para a linguagem **EV (Expressões com Variáveis)**.  
O compilador lê um arquivo-fonte contendo declarações de variáveis e uma expressão final, constrói uma AST, realiza verificação semântica e gera código assembly x86-64.

O programa gerado calcula o valor da expressão final e imprime o resultado utilizando o runtime fornecido.

---

## Integrantes

- **ÊMILLY EDUARDA CAROLINY SILVA** – 20220166942
- **LUIZ MANOEL BARBOSA RAMALHO** – 20220096614
- **REUBEN LISBOA RAMALHO CLAUDINO** – 20210024602
- **VICTOR PESSOA OLIVEIRA ORTINS** – 20210024667

---

## Estrutura do projeto

No diretório do projeto, espera-se uma organização semelhante a esta:

```text
.
├── ev_compiler.c
├── runtime.s
├── README.md
├── testes/
│   └── exemplo.ev
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
````

### Função de cada arquivo

* `ev_compiler.c` — ponto de entrada do compilador e geração do `output.s`
* `src/ast.*` — estruturas da árvore sintática abstrata
* `src/lexical.*` — analisador léxico
* `src/parser.*` — analisador sintático
* `src/semantic.*` — análise semântica
* `src/utils.*` — funções utilitárias
* `runtime.s` — runtime fornecido pelo professor

---

## Gramática da linguagem EV

A linguagem EV segue a gramática abaixo:

```text
<programa> ::= <decl>* <result>
<decl>     ::= <ident> '=' <exp> ';'
<result>   ::= '=' <exp>
<exp>      ::= <exp_m> (('+' | '-') <exp_m>)*
<exp_m>    ::= <prim> (('*' | '/') <prim>)*
<prim>     ::= <num> | <ident> | '(' <exp> ')'
```

### Exemplo de programa válido

```text
x = (7 + 4) * 12;
y = x * 3 + 11;
= (x * y) + (x * 11) + (y * 13)
```

---

## Funcionalidades implementadas

O compilador realiza as seguintes etapas:

1. **Análise léxica**

   * reconhece inteiros
   * reconhece identificadores
   * reconhece operadores `+`, `-`, `*`, `/`
   * reconhece `=`, `;`, `(` e `)`

2. **Análise sintática**

   * constrói a AST do programa
   * respeita precedência entre operadores
   * respeita associatividade à esquerda

3. **Análise semântica**

   * verifica uso de variável antes da declaração
   * verifica redeclaração de variável

4. **Geração de código**

   * cria variáveis na seção `.bss`
   * gera código para declarações
   * gera código para a expressão final
   * imprime o resultado com `imprime_num`

---

## Como compilar

Compile o compilador com:

```bash
gcc -Wall -Wextra -std=c11 ev_compiler.c src/ast.c src/lexical.c src/parser.c src/semantic.c src/utils.c -o ev_compiler
```

Isso irá gerar o executável:

```text
ev_compiler
```

---

## Como executar

Execute o compilador passando como argumento um arquivo `.ev`:

```bash
./ev_compiler testes/exemplo.ev
```

Após essa execução, será gerado automaticamente o arquivo:

```text
output.s
```

Esse arquivo contém o código assembly x86-64 correspondente ao programa EV, já incluindo o `runtime.s`.

---

## Como montar e linkar o assembly gerado

Como o arquivo `output.s` já contém a diretiva:

```asm
.include "runtime.s"
```

não é necessário montar o runtime separadamente.

Basta executar:

```bash
as -o output.o output.s
ld -o programa output.o
```

---

## Como executar o programa final

Depois de montar e linkar, execute:

```bash
./programa
```

O valor da expressão final será impresso no terminal.

---

## Exemplo completo

### Arquivo `teste.ev`

```text
x = 10;
y = x + 5;
= y * 2
```

### Comandos

```bash
./ev_compiler teste.ev
as -o output.o output.s
ld -o programa output.o
./programa
```

### Saída esperada

```text
30
```

---

## Exemplo de erro semântico

O programa abaixo é inválido:

```text
x = y + 1;
y = 10;
= x + y
```

Nesse caso, o compilador deve acusar erro semântico, pois a variável `y` foi usada antes de ser declarada.

Outro exemplo inválido:

```text
x = 10;
x = 20;
= x
```

Nesse caso, o compilador deve acusar redeclaração de variável.

---

## Observações importantes

* O resultado final de cada expressão é deixado no registrador `%rax`.
* A estratégia de geração usa pilha (`push`/`pop`) para preservar a ordem correta das operações.
* As variáveis são alocadas na seção `.bss` com `.lcomm`.
* A impressão é feita pela função `imprime_num`, definida em `runtime.s`.
* Não deve ser feito o link manual de `runtime.o` quando se usa `.include "runtime.s"`.
* Os identificadores devem começar com letra.
* A linguagem aceita:

  * inteiros positivos
  * identificadores
  * operadores binários `+`, `-`, `*`, `/`
  * parênteses para agrupamento
  * declarações com `;`
  * uma expressão final iniciada por `=`

---

## Ambiente de desenvolvimento

* Sistema operacional: Linux (x86-64)
* Compilador C: `gcc`
* Montador e linker: `as` e `ld` (GNU binutils)

---

## Extensão dos arquivos de entrada

Os arquivos-fonte desta atividade usam a extensão:

```text
.ev
```

Exemplo:

```text
programa.ev
```

---

## Resumo

Este compilador implementa uma pequena linguagem com variáveis, realizando:

* análise léxica
* análise sintática
* análise semântica
* geração de assembly x86-64

Ao final, ele produz um executável que avalia a expressão final do programa e imprime o resultado.