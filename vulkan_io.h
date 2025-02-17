//
// Created by brymher on 26/01/25.
//

#ifndef VULKAN_IO_H
#define VULKAN_IO_H

#include <vulkan/vulkan.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include "array.h"
#include "io.h"


void readFile(const char *filename,  Uint32SizedMutableArray *shader) {
    FILE *file = fopen(filename, "rb");

    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        exit(1);
    } else printLn("Opened file: %s\n", filename);

    fseek(file, 0, SEEK_END);
    const size_t bufferSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    shader->items = malloc(bufferSize);
    shader->size = bufferSize;
    shader->count = (bufferSize / sizeof(char));

    fread(shader->items, 1, shader->size, file);
    fclose(file);
}
#endif //VULKAN_IO_H
