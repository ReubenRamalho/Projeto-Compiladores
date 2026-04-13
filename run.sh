gcc -Wall -Wextra -std=c11 fun_compiler.c src/*.c -o fun_compiler
./fun_compiler testes/teste6.fun
as -o output.o output.s
ld -o programa output.o
./programa