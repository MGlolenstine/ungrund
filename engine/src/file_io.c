#include "ungrund.h"
#include <stdio.h>
#include <stdlib.h>

char* ug_read_file(const char* filepath) {
    if (!filepath) {
        return NULL;
    }

    FILE* file = fopen(filepath, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filepath);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, size, file);
    buffer[size] = '\0';

    fclose(file);
    return buffer;
}

unsigned char* ug_read_binary_file(const char* filepath, size_t* out_size) {
    if (!filepath || !out_size) {
        return NULL;
    }

    FILE* file = fopen(filepath, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filepath);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *out_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char* buffer = (unsigned char*)malloc(*out_size);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, *out_size, file);

    fclose(file);
    return buffer;
}

