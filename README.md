# Analisador Léxico EC1 — Expressões Constantes

Este projeto implementa o analisador léxico (scanner) para a linguagem **EC1 (Expressões Constantes 1)**.  
O programa lê um arquivo `.ci` contendo uma expressão EC1, varre o texto, ignora espaços em branco e agrupa os caracteres em uma sequência de tokens, imprimindo-os no terminal. Ele também é capaz de detectar caracteres inválidos e reportar erros léxicos.

---

## Integrantes
- **ÊMILLY EDUARDA CAROLINY SILVA** – 20220166942
- **LUIZ MANOEL BARBOSA RAMALHO** – 20220096614  
- **REUBEN LISBOA RAMALHO CLAUDINO** – 20210024602  
- **VICTOR PESSOA OLIVEIRA ORTINS** – 20210024667  

---

## Estrutura

No diretório do projeto existem os seguintes arquivos e pastas:

- `ec1_lexer.c` — código-fonte principal do analisador léxico  
- `src/` — diretório contendo os arquivos `.c` e `.h` referentes ao Analisador Léxico (`lexical`) e Utilitários (`utils`).
- `arquivo.ci` — arquivo de entrada contendo a expressão EC1  

---

## Como rodar

### 1) Compilar o analisador léxico

Primeiro, compile o código usando o `gcc` (note que apenas os arquivos necessários para a análise léxica são compilados):

```bash
gcc -Wall -Wextra src/lexical.c src/utils.c ec1_lexer.c -o ec1_lexer

```

Isso irá gerar o executável `ec1_lexer`.

---

### 2) Executar o analisador léxico

Em seguida, execute o programa passando como argumento o arquivo `.ci` que contém a expressão EC1:

```bash
./ec1_lexer testes/arquivo.ci

```

A saída será a sequência de tokens encontrados no arquivo, formatados como:
`<Tipo, "Lexema", Posicao>`

---

## Observações importantes

* O programa varre o código fonte e classifica a entrada nas seguintes classes léxicas:
* **Número:** Constantes inteiras.
* **Pontuação:** Parênteses esquerdos e direitos `( )`.
* **Operadores:** `+`, `-`, `*`, `/`.


* Qualquer caractere fora deste conjunto gera uma interrupção com a mensagem de `Erro léxico na posição X`.

Exemplo de entrada (`arquivo.ci`):

```
(33 + 12)

```

Saída esperada:

```
<ParenEsq, "(", 0>
<Numero, "33", 1>
<Soma, "+", 4>
<Numero, "12", 7>
<ParenDir, ")", 9>

```