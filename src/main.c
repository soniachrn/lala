#include "parser.h"
#include "vm.h"


typedef enum {
    LALA_HELP,
    LALA_COMPILE,
    LALA_EXECUTE,
    LALA_INTERPRET,
    LALA_INVALID,
} LalaMode;

typedef struct {
    LalaMode mode;
    const char* input_filename;
    const char* output_filename;
} LalaArguments;


static LalaMode parseMode(const char* modeStr);
static LalaArguments parseArguments(int argc, const char* argv[]);

static char* readFile(const char* filename);
static void writeBytesToFile(const char* filename, uint8_t* bytes, size_t length);

static void help(LalaArguments arguments);
static void compile(LalaArguments arguments);
static void execute(LalaArguments arguments);
static void lalaInterpret(LalaArguments arguments);


int main(int argc, const char* argv[]) {

    LalaArguments arguments = parseArguments(argc, argv);

    switch (arguments.mode) {
        case LALA_HELP:      help(arguments);          break;
        case LALA_COMPILE:   compile(arguments);       break;
        case LALA_EXECUTE:   execute(arguments);       break;
        case LALA_INTERPRET: lalaInterpret(arguments); break;
        case LALA_INVALID:
            fprintf(stderr, "Run 'lala help' for help\n");
            exit(1);
    }

    return 0;
}


static LalaMode parseMode(const char* modeStr) {
    if (strcmp(modeStr, "h") == 0 || 
        strcmp(modeStr, "help") == 0
    ) {
        return LALA_HELP;
    }

    else if (
        strcmp(modeStr, "c") == 0 ||
        strcmp(modeStr, "compile") == 0
    ) {
        return LALA_COMPILE;
    }

    else if (
        strcmp(modeStr, "e") == 0 ||
        strcmp(modeStr, "execute") == 0
    ) {
        return LALA_EXECUTE;
    }

    else if (
        strcmp(modeStr, "i") == 0 ||
        strcmp(modeStr, "interpret") == 0
    ) {
        return LALA_INTERPRET;
    }

    else {
        fprintf(stderr, "Invalid syntax.\n");
        return LALA_INVALID;
    }
}

static LalaArguments parseArguments(int argc, const char* argv[]) {
    LalaArguments arguments;

    arguments.mode = argc > 1 ? parseMode(argv[1]) : LALA_INVALID;

    switch (arguments.mode) {
        case LALA_HELP:
        case LALA_INVALID:
            break;
        case LALA_COMPILE:
            if (argc == 4) {
                arguments.input_filename = argv[2];
                arguments.output_filename = argv[3];
            } else {
                fprintf(stderr,
                    "Expected 2 arguments in compile mode: "
                    "input and output file names. "
                    "Got %d arguments.\n"
                    "%s %s <input file name> <output file name>\n", 
                    argc - 2, argv[0], argv[1]
                );
                arguments.mode = LALA_INVALID;
            }
            break;
        case LALA_EXECUTE:
            if (argc == 3) {
                arguments.input_filename = argv[2];
            } else {
                fprintf(stderr,
                    "Expected 1 argument in execute mode: "
                    "input file name. Got %d arguments.\n"
                    "%s %s <input file name>\n",
                    argc - 2, argv[0], argv[1]
                );
                arguments.mode = LALA_INVALID;
            }
            break;
        case LALA_INTERPRET:
            if (argc == 3) {
                arguments.input_filename = argv[2];
            } else {
                fprintf(stderr,
                    "Expected 1 argument in interpret mode: "
                    "input file name. Got %d arguments.\n"
                    "%s %s <input file name>\n",
                    argc - 2, argv[0], argv[1]
                );
                arguments.mode = LALA_INVALID;
            }
            break;
    }

    return arguments;
}

static char* readFile(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Couldn't open file '%s'.\n", filename);
        exit(1);
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = (size_t)ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Couldn't allocate memory for file '%s'.\n", filename);
        exit(1);
    }

    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    if (bytes_read < file_size) {
        fprintf(stderr, "Couldn't read file '%s'.\n", filename);
        exit(1);
    }

    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}

static void writeBytesToFile(const char* filename, uint8_t* bytes, size_t length) {
    FILE* file = fopen(filename, "wb");

    fwrite(bytes, 1, length, file);

    fclose(file);
}


static void help(LalaArguments arguments) {
    (void)arguments;

    printf("Lala language.\n");
    printf("lala <command> [<args>]\n\n");

    printf("Available commands:\n");
    printf("  help - Print this message\n");
    printf("  compile <lala file> <lalaby output file> - Compile lala source file into lalaby bytecode file.\n");
    printf("  execute <lalaby file> - Execute the given lalaby bytecode file.\n");
    printf("  interpret <lala file> - Compile the given lala source file and execute it right away.\n");
}

static void compile(LalaArguments arguments) {
    char* source = readFile(arguments.input_filename);
    
    Lexer lexer;
    initLexer(&lexer, source);

    Stack bytecode;
    initStack(&bytecode);

    Parser parser;
    initParser(&parser, &lexer, &bytecode);

    parse(&parser);

    writeBytesToFile(
        arguments.output_filename,
        bytecode.stack,
        (size_t)(bytecode.stack_top - bytecode.stack)
    );

    freeParser(&parser);
    freeStack(&bytecode);
    freeLexer(&lexer);

    free(source);
}

static void execute(LalaArguments arguments) {
    uint8_t* source = (uint8_t*)readFile(arguments.input_filename);

    VM vm;
    initVM(&vm, source);

    interpret(&vm);

    freeVM(&vm);

    free(source);
}

static void lalaInterpret(LalaArguments arguments) {
    (void)arguments;
    fprintf(stderr, "Not implemented yet.\n");
}

