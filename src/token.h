// ❤️

#ifndef lala_token_h
#define lala_token_h


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
    TOKEN_QUESTION,
    TOKEN_RBRACE,
    TOKEN_RBRACKET,
    TOKEN_RPAREN,
    TOKEN_SLASH,
    TOKEN_SLASH_EQUAL,
    TOKEN_STAR,
    TOKEN_STAR_EQUAL,
    
    // Words
    TOKEN_AND,
    TOKEN_ASSERT,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_ELSE,
    TOKEN_ENUM,
    TOKEN_FALSE,
    TOKEN_FOR,
    TOKEN_FUNCTION,
    TOKEN_IF,
    TOKEN_IN,
    TOKEN_MUTABLE,
    TOKEN_OR,
    TOKEN_PREDICATE,
    TOKEN_PRINT,
    TOKEN_RETURN,
    TOKEN_STRUCTURE,
    TOKEN_TRUE,
    TOKEN_VAR,
    TOKEN_WHILE,

    // Complex
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_IDENTIFIER,

} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    uint8_t length;
    uint16_t line;
    uint8_t symbol;
} Token;


#endif

