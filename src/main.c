#include "parser.h"
#include "vm.h"


#include <assert.h>

#include "path.h"


typedef enum {
    LALA_HELP,
    LALA_COMPILE,
    LALA_EXECUTE,
    LALA_INTERPRET,
    LALA_DISASSEMBLE,
    LALA_INVALID,
} LalaMode;

typedef struct {
    LalaMode mode;
    const char* input_filename;
    const char* output_filename;
} LalaArguments;

typedef struct {
    uint8_t version[3];
    size_t constants_offset;
    size_t constants_length;
    size_t program_offset;
    size_t program_length;
} LalabyHeader;


#define LALABY_HEADER_SIZE (3 * sizeof(uint8_t) + 4 * sizeof(size_t))


static LalaMode parseMode(const char* modeStr);
static LalaArguments parseArguments(int argc, const char* argv[]);

static void fillLalabyHeader(LalabyHeader* header, const Parser* parser);
static void   serializeLalabyHeader(FILE* file, const LalabyHeader* header);
static void deserializeLalabyHeader(const uint8_t* source, LalabyHeader* header);

// static char* readFile(const char* filename);

static void help(LalaArguments arguments);
static void compile(LalaArguments arguments);
static void execute(LalaArguments arguments);
static void lalaInterpret(LalaArguments arguments);
static void disassemble(LalaArguments arguments);


int main(int argc, const char* argv[]) {

    LalaArguments arguments = parseArguments(argc, argv);

    switch (arguments.mode) {
        case LALA_HELP:        help(arguments);          break;
        case LALA_COMPILE:     compile(arguments);       break;
        case LALA_EXECUTE:     execute(arguments);       break;
        case LALA_INTERPRET:   lalaInterpret(arguments); break;
        case LALA_DISASSEMBLE: disassemble(arguments);   break;
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

    else if (
        strcmp(modeStr, "d") == 0 ||
        strcmp(modeStr, "disassemble") == 0
    ) {
        return LALA_DISASSEMBLE;
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
        case LALA_DISASSEMBLE:
            if (argc == 3) {
                arguments.input_filename = argv[2];
            } else {
                fprintf(stderr,
                    "Expected 1 argument in disassemble mode: "
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

// static char* readFile(const char* filename) {
//     FILE* file = fopen(filename, "rb");
//     if (file == NULL) {
//         fprintf(stderr, "Couldn't open file '%s'.\n", filename);
//         exit(1);
//     }
// 
//     fseek(file, 0L, SEEK_END);
//     size_t file_size = (size_t)ftell(file);
//     rewind(file);
// 
//     char* buffer = (char*)malloc(file_size + 1);
//     if (buffer == NULL) {
//         fprintf(stderr, "Couldn't allocate memory for file '%s'.\n", filename);
//         exit(1);
//     }
// 
//     size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
//     if (bytes_read < file_size) {
//         fprintf(stderr, "Couldn't read file '%s'.\n", filename);
//         exit(1);
//     }
// 
//     buffer[bytes_read] = '\0';
// 
//     fclose(file);
//     return buffer;
// }

static void fillLalabyHeader(LalabyHeader* header, const Parser* parser) {
    assert(header);
    assert(parser);

    header->version[0] = 0;
    header->version[1] = 0;
    header->version[2] = 2;

    header->constants_offset = LALABY_HEADER_SIZE;
    header->constants_length = getConstantSectionSize(&parser->constants);
    header->program_offset   = header->constants_offset + header->constants_length;
    header->program_length   = (size_t)(parser->chunk->stack_top - parser->chunk->stack);
}

static void serializeLalabyHeader(FILE* file, const LalabyHeader* header) {
    assert(file);
    assert(header);

    fwrite(header->version, sizeof(uint8_t), 3, file);
    fwrite(&header->constants_offset, sizeof(size_t), 1, file);
    fwrite(&header->constants_length, sizeof(size_t), 1, file);
    fwrite(&header->program_offset, sizeof(size_t), 1, file);
    fwrite(&header->program_length, sizeof(size_t), 1, file);
}

static void deserializeLalabyHeader(const uint8_t* source, LalabyHeader* header) {
    assert(source);
    assert(header);

    header->version[0] = *source;
    source += sizeof(uint8_t);
    header->version[1] = *source;
    source += sizeof(uint8_t);
    header->version[2] = *source;
    source += sizeof(uint8_t);

    header->constants_offset = *(const size_t*)source;
    source += sizeof(size_t);
    header->constants_length = *(const size_t*)source;
    source += sizeof(size_t);
    header->program_offset = *(const size_t*)source;
    source += sizeof(size_t);
    header->program_length = *(const size_t*)source;
    source += sizeof(size_t);
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
    printf("  disassemble <lalaby file> - Disassemble the given lalaby bytecode file.\n");
}

static void compile(LalaArguments arguments) {
    char* source = NULL;
    if (readFileAndPrintErrors(arguments.input_filename, &source, stderr) != READ_FILE_SUCCESS) {
        exit(1);
    }
    // char* source = readFile(arguments.input_filename);
    
    // Init lexer, bytecode stack, parser
    Lexer lexer;
    Stack bytecode;
    Parser parser;
    initLexer(&lexer, source);
    initStack(&bytecode);
    initParser(&parser, &lexer, &bytecode);

    // Parse
    parse(&parser);

    if (!parser.had_error) {
        // Fill lalaby header
        LalabyHeader header;
        fillLalabyHeader(&header, &parser);

        // Open file
        FILE* file = fopen(arguments.output_filename, "wb");
        if (file == NULL) {
            fprintf(stderr, "Couldn't open file '%s'.\n", arguments.output_filename);
            exit(1);
        }
        
        // Write lalaby header, constants, program
        serializeLalabyHeader(file, &header);
        serializeConstants(file, &parser.constants);
        fwrite(bytecode.stack, 1, (size_t)(bytecode.stack_top - bytecode.stack), file);

        // Close file
        fclose(file);
    }

    // Free lexer, bytecode stack, parser
    freeParser(&parser);
    freeStack(&bytecode);
    freeLexer(&lexer);

    free(source);
}

static void execute(LalaArguments arguments) {
    uint8_t* source = NULL;
    if (readFileAndPrintErrors(arguments.input_filename, (char**)&source, stderr) != READ_FILE_SUCCESS) {
        exit(1);
    }
    // uint8_t* source = (uint8_t*)readFile(arguments.input_filename);

    LalabyHeader header;
    deserializeLalabyHeader(source, &header);

    uint8_t* constants_section = source + header.constants_offset;
    uint8_t* program = source + header.program_offset;

    Constants constants;
    deserializeConstants(constants_section, header.constants_length, &constants);

    VM vm;
    initVM(&vm, program, header.program_length, &constants);

    interpret(&vm);

    freeVM(&vm);

    free(source);
}

static void lalaInterpret(LalaArguments arguments) {
    (void)arguments;
    fprintf(stderr, "Not implemented yet.\n");
}

static void disassemble(LalaArguments arguments) {
    uint8_t* source = NULL;
    if (readFileAndPrintErrors(arguments.input_filename, (char**)&source, stderr) != READ_FILE_SUCCESS) {
        exit(1);
    }
    // uint8_t* source = (uint8_t*)readFile(arguments.input_filename);

    LalabyHeader header;
    deserializeLalabyHeader(source, &header);

    uint8_t* constants_section = source + header.constants_offset;
    uint8_t* program = source + header.program_offset;

    Constants constants;
    deserializeConstants(constants_section, header.constants_length, &constants);

    // header
    printf("–– HEADER (0-%ld)\n", LALABY_HEADER_SIZE - 1);
    printf("version %u.%u.%u\n", header.version[0], header.version[1], header.version[2]);
    printf("constants_offset = %ld\n", header.constants_offset);
    printf("constants_length = %ld\n", header.constants_length);
    printf("program_offset = %ld\n", header.program_offset);
    printf("program_length = %ld\n", header.program_length);
    printf("\n");

    // constants
    printf("–– CONSTANTS (%ld-%ld)\n", header.constants_offset, header.constants_offset + header.constants_length - 1);
    for (uint8_t i = 0; i < constants.count; ++i) {
        printf("[%u] = \"%.*s\"\n", i, constants.constants[i].length, constants.constants[i].value);
    }
    printf("\n");

    // program
    printf("–– PROGRAM (%ld-%ld)\n", header.program_offset, header.program_offset + header.program_length - 1);

    for (uint8_t* ip = program; ip < program + header.program_length; ) {
        printf("%2lx %-22s", ip - program, opCodeName((OpCode)*ip));

        switch (*ip++) {
            case OP_PUSH_BYTE:
            case OP_LOAD_CONSTANT:
                printf(" %u", *(uint8_t*)ip);
                ip += sizeof(uint8_t);
                break;

            case OP_PUSH_INT:
                printf(" %d", *(int32_t*)ip);
                ip += sizeof(int32_t);
                break;

            case OP_PUSH_FLOAT:
                printf(" %g", *(double*)ip);
                ip += sizeof(double);
                break;

            case OP_DEFINE_ON_HEAP:
                printf(" %lu", *(size_t*)ip);
                ip += sizeof(size_t);
                printf(" %u", *(uint8_t*)ip);
                ip += sizeof(uint8_t);
                break;

            case OP_PUSH_ADDRESS:
            case OP_POP_BYTES:

            case OP_GET_BYTE_FROM_HEAP:
            case OP_GET_INT_FROM_HEAP:
            case OP_GET_FLOAT_FROM_HEAP:
            case OP_GET_ADDRESS_FROM_HEAP:
            case OP_SET_BYTE_ON_HEAP:
            case OP_SET_INT_ON_HEAP:
            case OP_SET_FLOAT_ON_HEAP:
            case OP_SET_ADDRESS_ON_HEAP:

            case OP_GET_LOCAL_BYTE:
            case OP_GET_LOCAL_INT:
            case OP_GET_LOCAL_FLOAT:
            case OP_GET_LOCAL_ADDRESS:
            case OP_SET_LOCAL_BYTE:
            case OP_SET_LOCAL_INT:
            case OP_SET_LOCAL_FLOAT:
            case OP_SET_LOCAL_ADDRESS:
            case OP_GET_GLOBAL_BYTE:

            case OP_GET_GLOBAL_INT:
            case OP_GET_GLOBAL_FLOAT:
            case OP_GET_GLOBAL_ADDRESS:
            case OP_SET_GLOBAL_BYTE:
            case OP_SET_GLOBAL_INT:
            case OP_SET_GLOBAL_FLOAT:
            case OP_SET_GLOBAL_ADDRESS:

            case OP_CALL:
            case OP_JUMP:
            case OP_JUMP_IF_TRUE:
            case OP_JUMP_IF_FALSE:
                printf(" %p", (void*)*(size_t*)ip);
                ip += sizeof(size_t);
                break;

            default:
                break;
        }

        printf("\n");
    }
    printf("%2lx\n", header.program_length);

    free(source);
}

