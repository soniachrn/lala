#ifndef lala_parser_h
#define lala_parser_h


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "constant.h"
#include "lexer.h"
#include "op_code.h"
#include "path.h"
#include "scope.h"
#include "stack.h"
#include "token.h"
#include "value_type.h"


// ┌───────┐
// │ Types │
// └───────┘

typedef enum {
    INCLUDE_NOT_STARTED,
    INCLUDE_IN_PROCESS,
    INCLUDE_FINISHED,
} IncludeState;

typedef struct {
    HashMap includes;
    Lexer* lexer;
    Stack* chunk;

    Token previous;
    Token next;
    bool did_read_next;

    bool panic_mode;
    bool had_error;
    
    Scope* scope;
    Constants constants;

    Stack free_on_end;
} Parser;

typedef enum {
    PARSE_FILE_RECURSIVE_INCLUDE,
    PARSE_FILE_READ_FILE_ERROR,
    PARSE_FILE_SUCCESS,
} ParseFileResultType;

typedef struct {
    ParseFileResultType type;
    ReadFileResult read_file_result;
} ParseFileResult;

typedef struct {
    bool ends_with_return;
} StatementProperties;

typedef enum {
    EXPRESSION,
    EXPRESSION_STATEMENT,
} ExpressionKind;


// ┌───────────────────────┐
// │ Function declarations │
// └───────────────────────┘

void initParser(Parser* parser, Stack* chunk);
void freeParser(Parser* parser);
void dumpParser(const Parser* parser);
void fdumpParser(FILE* out, const Parser* parser, int padding);

ParseFileResult parseFile(Parser* parser, const char* file_path);
void parseString(Parser* parser, const char* source);
void parse(Parser* parser);
ValueType* parseExpression(Parser* parser);


#endif

