#ifndef lala_path_h
#define lala_path_h


#include <stdio.h>


#ifdef _WIN32
  #define PATH_SEPARATOR '\\'
#else
  #define PATH_SEPARATOR '/'
#endif


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

// Concatenates path_l, PATH_SEPARATOR and path_r.
// path_l is expected to be 0-terminated and is freed.
// path_r isn't expected to be 0-terminated and is not freed.
// A pointer to the allocated 0-terminated concatenation result is returned.
char* concatenatePath(
    char* path_l, 
    const char* path_r_start, 
    size_t path_r_length
);

// Concatenates path, dot and extension.
// path and extension are expected to be 0-terminated.
// path is freed.
// extension isn't freed.
// A pointer to the allocated 0-terminated concatenation result is returned.
char* addExtensionToPath(
    char* path,
    const char* extension
);


#endif

