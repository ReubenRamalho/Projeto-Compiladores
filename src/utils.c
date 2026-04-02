#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

void die(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    va_end(args);
    exit(1);
}

char *read_entire_file(const char *path) {
    FILE *f = fopen(path, "rb");
    long n;
    char *buf;
    size_t got;

    if (!f) die("Erro abrindo '%s': %s", path, strerror(errno));
    if (fseek(f, 0, SEEK_END) != 0) die("fseek falhou");
    n = ftell(f);
    if (n < 0) die("ftell falhou");
    rewind(f);
    buf = (char *)malloc((size_t)n + 1);
    if (!buf) die("Sem memória");
    got = fread(buf, 1, (size_t)n, f);
    fclose(f);
    if (got != (size_t)n) die("Leitura incompleta");
    buf[n] = '\0';
    return buf;
}

void emit(FILE *out, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    fputc('\n', out);
    va_end(args);
}
