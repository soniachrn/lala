#ifndef lala_parser_h
#define lala_parser_h


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "lexer.h"
#include "op_code.h"
#include "scope.h"
#include "stack.h"
#include "token.h"
#include "value_type.h"


// ┌───────┐
// │ Types │
// └───────┘

typedef struct {
    Lexer* lexer;
    Stack* chunk;

    Token previous;
    Token next;
    bool did_read_next;
    
    Scope* scope;
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

