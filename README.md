# Compilador Cmd — Linguagem com Comandos, Condicionais e Repetição

Este projeto implementa um compilador simples para a linguagem **Cmd**, uma evolução da linguagem da atividade anterior.  
O compilador lê um arquivo-fonte contendo declarações de variáveis, comandos e uma instrução de retorno, constrói uma AST, realiza verificação semântica e gera código assembly x86-64.

O programa gerado calcula o valor retornado pelo comando `return` e imprime o resultado utilizando o runtime fornecido.

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
├── cmd_compiler.c
├── runtime.s
├── README.md
├── testes/
│   └── exemplo.cmd
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

* `cmd_compiler.c` — ponto de entrada do compilador e geração do `output.s`
* `src/ast.*` — estruturas da árvore sintática abstrata
* `src/lexical.*` — analisador léxico
* `src/parser.*` — analisador sintático
* `src/semantic.*` — análise semântica
* `src/utils.*` — funções utilitárias
* `runtime.s` — runtime fornecido pelo professor

---

## Gramática da linguagem Cmd

A linguagem Cmd segue a estrutura geral abaixo:

```text
<programa> ::= <decl>* '{' <comando>* 'return' <exp> ';' '}'
```

Ela inclui:

* declarações de variáveis no início do programa
* bloco principal delimitado por `{` e `}`
* comandos de atribuição
* comandos condicionais `if ... else`
* comandos de repetição `while`
* comando final `return`

Também há suporte a:

* operadores aritméticos `+`, `-`, `*`, `/`
* operadores relacionais `<`, `>` e `==`
* parênteses para agrupamento

### Exemplo de programa válido

```text
x = 10;
y = 0;

{
    while (x > 0) {
        y = y + x;
        x = x - 1;
    }

    return y;
}
```

---

## Funcionalidades implementadas

O compilador realiza as seguintes etapas:

1. **Análise léxica**

   * reconhece inteiros
   * reconhece identificadores
   * reconhece palavras-chave `if`, `else`, `while` e `return`
   * reconhece operadores aritméticos `+`, `-`, `*`, `/`
   * reconhece operadores relacionais `<`, `>` e `==`
   * reconhece símbolos `=`, `;`, `(`, `)`, `{` e `}`

2. **Análise sintática**

   * constrói a AST completa do programa
   * reconhece declarações, comandos e retorno
   * respeita precedência entre operadores
   * respeita associatividade à esquerda

3. **Análise semântica**

   * verifica uso de variável antes da declaração
   * verifica redeclaração de variável
   * verifica atribuição em variável não declarada

4. **Geração de código**

   * cria variáveis na seção `.bss`
   * gera código para expressões aritméticas e relacionais
   * gera código para atribuições
   * gera código para estruturas `if/else`
   * gera código para estruturas `while`
   * gera código para o `return`
   * imprime o resultado com `imprime_num`

---

## Como compilar

Compile o compilador com:

```bash
gcc -Wall -Wextra -std=c11 cmd_compiler.c src/*.c -o cmd_compiler
```

Isso irá gerar o executável:

```text
cmd_compiler
```

---

## Como executar

Execute o compilador passando como argumento um arquivo `.cmd`:

```bash
./cmd_compiler testes/exemplo.cmd
```

Após essa execução, será gerado automaticamente o arquivo:

```text
output.s
```

Esse arquivo contém o código assembly x86-64 correspondente ao programa Cmd, já incluindo o `runtime.s`.

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

O valor retornado pelo comando `return` será impresso no terminal.

---

## Exemplo completo

### Arquivo `teste.cmd`

```text
x = 5;
y = 1;

{
    while (x > 1) {
        y = y * x;
        x = x - 1;
    }

    return y;
}
```

### Comandos

```bash
./cmd_compiler teste.cmd
as -o output.o output.s
ld -o programa output.o
./programa
```

### Saída esperada

```text
120
```

---

## Exemplo com comando condicional

```text
x = 10;
y = 20;

{
    if (x < y) {
        return x;
    } else {
        return y;
    }
}
```

Nesse caso, o programa imprime `10`.

---

## Exemplo de erro semântico

O programa abaixo é inválido:

```text
x = 10;

{
    y = x + 1;
    return y;
}
```

Nesse caso, o compilador deve acusar erro semântico, pois a variável `y` recebeu atribuição sem ter sido declarada.

Outro exemplo inválido:

```text
x = 10;
x = 20;

{
    return x;
}
```

Nesse caso, o compilador deve acusar redeclaração de variável.

Outro exemplo inválido:

```text
x = y + 1;

{
    return x;
}
```

Nesse caso, o compilador deve acusar uso de variável antes da declaração.

---

## Observações importantes

* O resultado final de cada expressão é deixado no registrador `%rax`.
* A estratégia de geração usa pilha (`push`/`pop`) para preservar a ordem correta das operações aritméticas.
* As variáveis são alocadas na seção `.bss` com `.lcomm`.
* A impressão é feita pela função `imprime_num`, definida em `runtime.s`.
* Não deve ser feito o link manual de `runtime.o` quando se usa `.include "runtime.s"`.
* Os identificadores devem começar com letra.
* A linguagem aceita:

  * inteiros positivos
  * identificadores
  * operadores aritméticos `+`, `-`, `*`, `/`
  * operadores relacionais `<`, `>` e `==`
  * parênteses para agrupamento
  * declarações com `;`
  * bloco delimitado por `{` e `}`
  * comandos `if`, `else`, `while`
  * comando `return`

---

## Ambiente de desenvolvimento

* Sistema operacional: Linux (x86-64)
* Compilador C: `gcc`
* Montador e linker: `as` e `ld` (GNU binutils)

---

## Extensão dos arquivos de entrada

Os arquivos-fonte desta atividade usam a extensão:

```text
.cmd
```

Exemplo:

```text
programa.cmd
```

---

## Resumo

Este compilador implementa uma pequena linguagem imperativa com variáveis, comandos e controle de fluxo, realizando:

* análise léxica
* análise sintática
* análise semântica
* geração de assembly x86-64

Ao final, ele produz um executável que avalia o programa, executa seus comandos e imprime o valor retornado.