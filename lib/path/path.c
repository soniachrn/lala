#include "path.h"


#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


ReadFileResult readFile(
    const char* file_path,
    char** buffer,
    size_t* length
) {
    assert(file_path);

    FILE* file = fopen(file_path, "rb");
    if (file == NULL) {
        // fprintf(stderr, "Couldn't open file '%s'.\n", file_path);
        return READ_FILE_COULD_NOT_OPEN_FILE;
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = (size_t)ftell(file);
    rewind(file);

    *buffer = (char*)malloc(file_size + 1);
    if (*buffer == NULL) {
        // fprintf(stderr, "Couldn't allocate memory for file '%s'.\n", file_path);
        return READ_FILE_COULD_NOT_ALLOCATE_MEMORY_FOR_FILE;
    }

    size_t bytes_read = fread(*buffer, sizeof(char), file_size, file);
    if (bytes_read < file_size) {
        free(*buffer);
        // fprintf(stderr, "Couldn't read file '%s'.\n", file_path);
        return READ_FILE_COULD_NOT_READ_FILE;
    }

    (*buffer)[bytes_read] = '\0';
    *length = bytes_read;

    fclose(file);
    return READ_FILE_SUCCESS;
}

ReadFileResult readFileAndPrintErrors(
    const char* file_path,
    char** buffer,
    size_t* length,
    FILE* out
) {
    ReadFileResult result = readFile(file_path, buffer, length);
    if (result != READ_FILE_SUCCESS) {
        fprintf(
            out,
            "%s %s\n",
            getReadFileResultErrorMessage(result),
            file_path
        );
    }
    return result;
}

const char* getReadFileResultErrorMessage(ReadFileResult result) {
    switch (result) {
        case READ_FILE_COULD_NOT_OPEN_FILE:
            return "Couldn't open file";
        case READ_FILE_COULD_NOT_ALLOCATE_MEMORY_FOR_FILE:
            return "Couldn't allocate memory to read file";
        case READ_FILE_COULD_NOT_READ_FILE:
            return "Couldn't read file";
        case READ_FILE_SUCCESS:
        default:
            assert(false);
    }
}

char* concatenatePath(
    char* path_l, 
    const char* path_r_start, 
    size_t path_r_length
) {
    assert(path_l);
    assert(path_r_start);

    size_t path_l_length = strlen(path_l);
    char* path = realloc(path_l, path_l_length + 1 + path_r_length + 1);
    path[path_l_length] = PATH_SEPARATOR;
    memcpy(path + path_l_length + 1, path_r_start, path_r_length);
    path[path_l_length + 1 + path_r_length] = '\0';
    return path;
}

char* addExtensionToPath(
    char* path,
    const char* extension
) {
    assert(path);
    assert(extension);

    size_t path_length = strlen(path);
    size_t extension_length = strlen(extension);
    path = realloc(path, path_length + extension_length + 2);
    path[path_length] = '.';
    memcpy(path + path_length + 1, extension, extension_length);
    path[path_length + 1 + extension_length] = '\0';
    return path;
}

