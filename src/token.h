// ❤️

#ifndef lala_token_h
#define lala_token_h


#include <stdint.h>


// ┌───────┐
// │ Types │
// └───────┘

typedef enum {

    // Utility
    TOKEN_END,
    TOKEN_ERROR,

    // Symbols
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_EXCLAMATION,
    TOKEN_EXCLAMATION_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LBRACE,
    TOKEN_LBRACKET,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_LPAREN,
    TOKEN_MINUS,
    TOKEN_MINUS_EQUAL,
    TOKEN_PERCENT,
    TOKEN_PERCENT_EQUAL,
    TOKEN_PLUS,
    TOKEN_PLUS_EQUAL,
    TOKEN_RBRACE,
    TOKEN_RBRACKET,
    TOKEN_RPAREN,
    TOKEN_SEMICOLON,
    TOKEN_SLASH,
    TOKEN_SLASH_EQUAL,
    TOKEN_STAR,
    TOKEN_STAR_EQUAL,
    
    // Keyords
    TOKEN_AND,
    TOKEN_ASSERT,
    TOKEN_BOOL,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_DO,
    TOKEN_ELSE,
    TOKEN_ENUM,
    TOKEN_FALSE,
    TOKEN_FLOAT,
    TOKEN_FOR,
    TOKEN_FUNCTION,
    TOKEN_IF,
    TOKEN_IN,
    TOKEN_INCLUDE,
    TOKEN_INT,
    TOKEN_MUTABLE,
    TOKEN_OR,
    TOKEN_PREDICATE,
    TOKEN_PRINT,
    TOKEN_READ,
    TOKEN_RETURN,
    TOKEN_STRING,
    TOKEN_STRUCTURE,
    TOKEN_TRUE,
    TOKEN_VAR,
    TOKEN_VOID,
    TOKEN_WHILE,

    // Complex
    TOKEN_INTEGER_VALUE,
    TOKEN_FLOAT_VALUE,
    TOKEN_STRING_VALUE,
    TOKEN_IDENTIFIER,

} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    uint8_t length;
    uint16_t line;
    uint8_t symbol;
} Token;


// ┌───────────┐
// │ Functions │
// └───────────┘

const char* tokenTypeName(TokenType type);


#endif

