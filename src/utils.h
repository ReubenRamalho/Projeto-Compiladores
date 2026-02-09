#pragma once

#include <stdio.h>

void die(const char *fmt, ...);

char *read_entire_file(const char *path);

void emit(FILE *out, const char *fmt, ...);