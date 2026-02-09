# Compilador EC1 — Expressões Constantes

Este projeto implementa um compilador simples para a linguagem **EC1 (Expressões Constantes 1)**.  
O compilador lê um arquivo `.ci` contendo uma expressão EC1, gera código assembly x86-64 e executa o programa resultante utilizando um runtime fornecido.

---

## Integrantes
- **ÊMILLY EDUARDA CAROLINY SILVA** – 20220166942
- **LUIZ MANOEL BARBOSA RAMALHO** – 20220096614  
- **REUBEN LISBOA RAMALHO CLAUDINO** – 20210024602  
- **VICTOR PESSOA OLIVEIRA ORTINS** – 20210024667  

---

## Estrutura esperada

No diretório do projeto devem existir os seguintes arquivos:

- `ec1_compiler.c` — código-fonte do compilador EC1  
- `runtime.s` — runtime fornecido pelo professor  
- `arquivo.ci` — arquivo de entrada contendo a expressão EC1  

---

## Como rodar

### 1) Compilar o compilador

Primeiro, compile o código do compilador EC1 usando o `gcc`:

```bash
gcc -Wall -Wextra src/*.c ec1_compiler.c -o ec1_compiler
```

Isso irá gerar o executável `ec1_compiler`.

---

### 2) Executar o compilador

Em seguida, execute o compilador passando como argumento o arquivo `.ci` que contém a expressão EC1:

```bash
./ec1_compiler testes/arquivo.ci
```

Após essa execução, será gerado automaticamente o arquivo:

```
output.s
```

Esse arquivo contém o código assembly x86-64 correspondente à expressão EC1, já incluindo o `runtime.s`.

---

### 3) Montar e linkar o código assembly

Como o arquivo `output.s` já contém a diretiva:

```asm
.include "runtime.s"
```

não é necessário montar o runtime separadamente. Basta executar:

```bash
as -o output.o output.s
ld -o programa output.o
```

---

### 4) Executar o programa final

Por fim, execute o programa gerado:

```bash
./programa
```

O resultado da expressão EC1 será impresso no terminal.

---

## Observações importantes

- O compilador gera código que deixa o valor final da expressão no registrador `%rax`.
- A impressão do resultado é feita pela função `imprime_num`, definida no `runtime.s`.
- Não deve ser feito o link manual de `runtime.o` quando se utiliza `.include "runtime.s"`, pois isso causaria duplicação de símbolos.
- A linguagem EC1 aceita apenas:
  - literais inteiros positivos
  - operadores binários `+`, `-`, `*`, `/`
  - expressões totalmente parentizadas

Exemplo de expressão válida:

```
( (2 + 3) * 10 )
```

---

## Ambiente de desenvolvimento

- Sistema operacional: Linux (x86-64)
- Compilador: `gcc`
- Montador e linker: `as` e `ld` (GNU binutils)
