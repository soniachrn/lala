#include "path.h"


#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>


ReadFileResult readFile(const char* file_path, char** buffer) {
    assert(file_path);

    FILE* file = fopen(file_path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Couldn't open file '%s'.\n", file_path);
        return READ_FILE_COULD_NOT_OPEN_FILE;
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = (size_t)ftell(file);
    rewind(file);

    *buffer = (char*)malloc(file_size + 1);
    if (*buffer == NULL) {
        fprintf(stderr, "Couldn't allocate memory for file '%s'.\n", file_path);
        return READ_FILE_COULD_NOT_ALLOCATE_MEMORY_FOR_FILE;
    }

    size_t bytes_read = fread(*buffer, sizeof(char), file_size, file);
    if (bytes_read < file_size) {
        fprintf(stderr, "Couldn't read file '%s'.\n", file_path);
        return READ_FILE_COULD_NOT_READ_FILE;
    }

    (*buffer)[bytes_read] = '\0';

    fclose(file);
    return READ_FILE_SUCCESS;
}

ReadFileResult readFileAndPrintErrors(
    const char* file_path,
    char** buffer,
    FILE* out
) {
    ReadFileResult result = readFile(file_path, buffer);
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
            return "Couldn't read file";
        case READ_FILE_COULD_NOT_READ_FILE:
            return "Couldn't read file";
        case READ_FILE_SUCCESS:
        default:
            assert(false);
    }
}

