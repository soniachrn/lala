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

    // Pointers to the start of each line.
    const char* lines[65536]; // max uint16_t
} Lexer;


// ┌───────────────────────┐
// │ Function declarations │
// └───────────────────────┘

void initLexer(Lexer* lexer, const char* input);
void freeLexer(Lexer* lexer);
void dumpLexer(const Lexer* lexer);
void fdumpLexer(FILE* out, const Lexer* lexer, int padding);

uint8_t fprintLine(FILE* out, Lexer* lexer, uint16_t line);

Token readToken(Lexer* lexer);


#endif

