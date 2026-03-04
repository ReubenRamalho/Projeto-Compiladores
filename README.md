# Interpretador EC1 — Expressões Constantes

Este projeto implementa um analisador sintático e interpretador simples para a linguagem **EC1 (Expressões Constantes 1)**.

O programa lê um arquivo `.ci` contendo uma expressão EC1, constrói a Árvore de Sintaxe Abstrata (AST) , imprime a estrutura da árvore e calcula o resultado final diretamente utilizando um interpretador de varredura de árvore (*tree-walking interpreter*).

---

## Integrantes

* **ÊMILLY EDUARDA CAROLINY SILVA** – 20220166942
* **LUIZ MANOEL BARBOSA RAMALHO** – 20220096614
* **REUBEN LISBOA RAMALHO CLAUDINO** – 20210024602
* **VICTOR PESSOA OLIVEIRA ORTINS** – 20210024667

---

## Estrutura esperada

No diretório do projeto devem existir os seguintes arquivos e pastas:

* `ec1_interpreter.c` — código-fonte principal do interpretador EC1
* `src/` — diretório contendo os arquivos `.c` e `.h` da AST, Analisador Léxico, Analisador Sintático e Utilitários
* `arquivo.ci` — arquivo de entrada contendo a expressão EC1

---

## Como rodar

### 1) Compilar o interpretador

Primeiro, compile o código do interpretador EC1 usando o `gcc`:

```bash
gcc -Wall -Wextra src/*.c ec1_interpreter.c -o ec1_interpreter

```

Isso irá gerar o executável `ec1_interpreter`.

---

### 2) Executar o interpretador

Em seguida, execute o programa passando como argumento o arquivo `.ci` que contém a expressão EC1:

```bash
./ec1_interpreter testes/arquivo.ci

```

A árvore sintática lida e o resultado da avaliação matemática da expressão EC1 serão impressos diretamente no terminal.

---

## Observações importantes

* O programa executa a expressão sem traduzi-la para outra linguagem, avaliando o resultado recursivamente direto dos nós da AST.


* A linguagem EC1 aceita apenas:


* literais inteiros 


* operadores binários `+`, `-`, `*`, `/` 


* expressões totalmente parentizadas 





Exemplo de expressão válida:

```
( (2 + 3) * 10 )

```

---

## Ambiente de desenvolvimento

* Sistema operacional: Linux (x86-64)
* Compilador: `gcc`