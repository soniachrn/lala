#ifndef lala_path_h
#define lala_path_h


#include <stdio.h>


typedef enum {
    READ_FILE_COULD_NOT_OPEN_FILE,
    READ_FILE_COULD_NOT_ALLOCATE_MEMORY_FOR_FILE,
    READ_FILE_COULD_NOT_READ_FILE,
    READ_FILE_SUCCESS,
} ReadFileResult;

ReadFileResult readFile(const char* file_path, char** buffer);
ReadFileResult readFileAndPrintErrors(
    const char* file_path,
    char** buffer,
    FILE* out
);

const char* getReadFileResultErrorMessage(ReadFileResult result);


#endif

