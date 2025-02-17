//
// Created by brymher on 01/01/25.
//

#ifndef LEARNING_IO_H
#define LEARNING_IO_H

#include <stdio.h>
#include <stdarg.h>

void print(FILE *__restrict stream, const char *__restrict format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stream, format, args);
    va_end(args);
    fflush(stream);
}

void printLn(const char *format, ...) {
    fprintf(stdout, "\n");
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    fflush(stdout);
}

void printErrLn(const char *format, ...) {

    fprintf(stderr, "\n");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fflush(stderr);
}

#endif //LEARNING_IO_H
