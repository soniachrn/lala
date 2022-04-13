#ifndef lala_parser_h
#define lala_parser_h


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "lexer.h"
#include "op_code.h"
#include "stack.h"
#include "token.h"


// ┌───────┐
// │ Types │
// └───────┘

typedef enum {
    VALUE_BOOL,
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_STRING,
    // VALUE_ARRAY,
    // VALUE_MAP,
    // VALUE_USER_DEFINED,
} ValueType;

typedef struct {
    Lexer* lexer;
    // Chunk* chunk;
    Stack* chunk;
    Token previous;
    Token next;
    bool did_read_next;
} Parser;


// ┌───────────────────────┐
// │ Function declarations │
// └───────────────────────┘

void initParser(Parser* parser, Lexer* lexer, Stack* chunk);
void freeParser(Parser* parser);
void dumpParser(const Parser* parser);
void fdumpParser(FILE* out, const Parser* parser, int padding);

void parse(Parser* parser);
ValueType parseExpression(Parser* parser);


#endif

