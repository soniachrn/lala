#ifndef lala_lexer_h
#define lala_lexer_h


#include <stdint.h>
#include <stdio.h>

#include "token.h"


// ┌───────┐
// │ Types │
// └───────┘

typedef struct {
    const char* token_start;
    const char* current;
    uint16_t line;
    uint8_t  token_start_symbol;
    uint8_t  symbol;
} Lexer;


// ┌───────────────────────┐
// │ Function declarations │
// └───────────────────────┘

void initLexer(Lexer* lexer, const char* input);
void freeLexer(Lexer* lexer);
void dumpLexer(const Lexer* lexer);
void fdumpLexer(FILE* out, const Lexer* lexer, int padding);

Token readToken(Lexer* lexer);


#endif

