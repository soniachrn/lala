#include "lexer.h"


#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"


// ┌────────┐
// │ Macros │
// └────────┘

#define VALIDATE_LEXER(lexer) \
    (                         \
        lexer              && \
        lexer->token_start && \
        lexer->current     && \
        lexer->line >= 1   && \
        lexer->symbol >= 1    \
    )

#define ASSERT_LEXER(lexer)                             \
    if (!VALIDATE_LEXER(lexer)) {                       \
        fprintf(stderr,                                 \
            "%s:%d, in %s:\nLexer assertion failed.\n", \
            __FILENAME__,                               \
            __LINE__,                                   \
            __FUNCTION_NAME__                           \
        );                                              \
        fdumpLexer(stderr, lexer, 0);                   \
        exit(1);                                        \
    }


// ┌──────────────────────────────┐
// │ Static function declarations │
// └──────────────────────────────┘

static char current(const Lexer* lexer);
static char next(const Lexer* lexer);
static bool isAtEnd(const Lexer* lexer);
static void advance(Lexer* lexer);
// If the next current equals c, advances and returns true; else, returns false
static bool match(Lexer* lexer, char c);

static void startNewToken(Lexer* lexer);
static Token makeToken(const Lexer* lexer, TokenType type);
static Token advanceAndMakeToken(Lexer* lexer, TokenType type);
static Token makeErrorToken(Lexer* lexer, const char* message);
static void fillErrorToken(Lexer* lexer, Token* error_token, const char* message);

static bool isLetter(char c);
static bool isDigit(char c);
static bool isAlphaNumeric(char c);     // [-a-zA-Z0-9]

// If encounters an unterminated multiline comment, fills error_token and returns false; else, returns true
static bool skipWhitespaceAndComments(Lexer* lexer, Token* error_token);
static Token string(Lexer* lexer);
static Token identifier(Lexer* lexer);
static TokenType identifierTokenType(const Lexer* lexer);
static TokenType tryMatchKeyword(const Lexer* lexer, const char* keyword, TokenType keywordTokenType);
static Token number(Lexer* lexer);


// ┌──────────────────────────┐
// │ Function implementations │
// └──────────────────────────┘

void initLexer(Lexer* lexer, const char* input) {
    assert(lexer);

    lexer->token_start = input;
    lexer->current = input;
    lexer->line = 1;
    lexer->token_start_symbol = 1;
    lexer->symbol = 1;
    lexer->lines[1] = input;

    ASSERT_LEXER(lexer);
}

void freeLexer(Lexer* lexer) {
    ASSERT_LEXER(lexer);

    lexer->token_start = NULL;
    lexer->current = NULL;
    lexer->line = 0;
    lexer->token_start_symbol = 0;
    lexer->symbol = 0;
}

void dumpLexer(const Lexer* lexer) {
    fdumpLexer(stdout, lexer, 0);
}

void fdumpLexer(FILE* out, const Lexer* lexer, int padding) {
    assert(out);

#define printf(...)                                \
    {                                              \
        if (padding > 0) {                         \
            fprintf(out, "%*s", padding * 2, " "); \
        }                                          \
        fprintf(out, __VA_ARGS__);                 \
    }

    if (!lexer) {
        fprintf(out, "Lexer *(NULL)\n");
    } else {
        fprintf(out, "Lexer *(%p) %s {\n",
            (const void*)lexer,
            VALIDATE_LEXER(lexer) ? "VALID" : "INVALID"
        );
        printf("  token_start = *(%p) '%c'\n", (const void*)lexer->token_start, lexer->token_start[0]);
        printf("  current = *(%p) '%c'\n", (const void*)lexer->current, lexer->current[0]);
        printf("  line = %d\n", lexer->line);
        printf("  token_start_symbol = %d\n", lexer->token_start_symbol);
        printf("  symbol = %d\n", lexer->symbol);
        printf("}\n");
    }

#undef printf
}

uint8_t fprintLine(FILE* out, Lexer* lexer, uint16_t line) {
    ASSERT_LEXER(lexer);

    assert(line <= lexer->line);
    const char* line_start = lexer->lines[line];
    const char* line_break = strchr(line_start, '\n');
    if (line_break) {
        fprintf(out, "%.*s\n", (int)(line_break - line_start), line_start);
        return (uint8_t)(line_break - line_start);
    } else {
        fprintf(out, "%s\n", line_start);
        return (uint8_t)strlen(line_start);
    }
}

Token readToken(Lexer* lexer) {
    ASSERT_LEXER(lexer);

    startNewToken(lexer);

    Token error_token;
    if (!skipWhitespaceAndComments(lexer, &error_token)) {
        return error_token;
    }

    startNewToken(lexer);

#define TOKEN_OR_TOKEN_EQUAL(tokenEqualType, tokenType) \
    { \
        advance(lexer); \
        return makeToken(lexer, match(lexer, '=') ? tokenEqualType : tokenType); \
    }

    char c = current(lexer);
    switch (c) {

        // One symbol
        case ':': return advanceAndMakeToken(lexer, TOKEN_COLON);
        case ',': return advanceAndMakeToken(lexer, TOKEN_COMMA);
        case '.': return advanceAndMakeToken(lexer, TOKEN_DOT);
        case '{': return advanceAndMakeToken(lexer, TOKEN_LBRACE);
        case '[': return advanceAndMakeToken(lexer, TOKEN_LBRACKET);
        case '(': return advanceAndMakeToken(lexer, TOKEN_LPAREN);
        case '}': return advanceAndMakeToken(lexer, TOKEN_RBRACE);
        case ']': return advanceAndMakeToken(lexer, TOKEN_RBRACKET);
        case ')': return advanceAndMakeToken(lexer, TOKEN_RPAREN);
        case ';': return advanceAndMakeToken(lexer, TOKEN_SEMICOLON);

        // One or two symbols
        case '=': TOKEN_OR_TOKEN_EQUAL(TOKEN_EQUAL_EQUAL,       TOKEN_EQUAL);
        case '!': TOKEN_OR_TOKEN_EQUAL(TOKEN_EXCLAMATION_EQUAL, TOKEN_EXCLAMATION);
        case '>': TOKEN_OR_TOKEN_EQUAL(TOKEN_GREATER_EQUAL,     TOKEN_GREATER);
        case '<': TOKEN_OR_TOKEN_EQUAL(TOKEN_LESS_EQUAL,        TOKEN_LESS);
        case '-': TOKEN_OR_TOKEN_EQUAL(TOKEN_MINUS_EQUAL,       TOKEN_MINUS);
        case '%': TOKEN_OR_TOKEN_EQUAL(TOKEN_PERCENT_EQUAL,     TOKEN_PERCENT);
        case '+': TOKEN_OR_TOKEN_EQUAL(TOKEN_PLUS_EQUAL,        TOKEN_PLUS);
        case '/': TOKEN_OR_TOKEN_EQUAL(TOKEN_SLASH_EQUAL,       TOKEN_SLASH);
        case '*': TOKEN_OR_TOKEN_EQUAL(TOKEN_STAR_EQUAL,        TOKEN_STAR);

        // String
        case '\'': return string(lexer);

        // End of input
        case '\0': return makeToken(lexer, TOKEN_END);

        // Alphanumeric
        default:
            if (isLetter(c)) {
                return identifier(lexer);
            } else if (isDigit(c)) {
                return number(lexer);
            }
            break;
    }

#undef TOKEN_OR_TOKEN_EQUAL

    return makeErrorToken(lexer, "Unexpected character.");
}


// ┌─────────────────────────────────┐
// │ Static function implementations │
// └─────────────────────────────────┘

static char current(const Lexer* lexer) {
    ASSERT_LEXER(lexer);
    return lexer->current[0];
}

static char next(const Lexer* lexer) {
    ASSERT_LEXER(lexer);
    return isAtEnd(lexer) ? '\0' : lexer->current[1];
}

static bool isAtEnd(const Lexer* lexer) {
    ASSERT_LEXER(lexer);
    return current(lexer) == '\0';
}

static void advance(Lexer* lexer) {
    ASSERT_LEXER(lexer);
    assert(!isAtEnd(lexer));

    if (current(lexer) == '\n') {
        lexer->line += 1;
        lexer->symbol = 1;
        lexer->lines[lexer->line] = lexer->current + 1;
    } else {
        lexer->symbol += 1;
    }
    lexer->current++;

    ASSERT_LEXER(lexer);
}

static bool match(Lexer* lexer, char expected) {
    ASSERT_LEXER(lexer);

    if (current(lexer) == expected) {
        advance(lexer);
        return true;
    } else {
        return false;
    }
}

static void startNewToken(Lexer* lexer) {
    ASSERT_LEXER(lexer);

    lexer->token_start = lexer->current;
    lexer->token_start_symbol = lexer->symbol;

    ASSERT_LEXER(lexer);
}

static Token makeToken(const Lexer* lexer, TokenType type) {
    ASSERT_LEXER(lexer);

    Token token;
    token.type = type;
    token.start = lexer->token_start;
    size_t length = (size_t)(lexer->current - lexer->token_start);
    assert(length < 256);
    token.length = (uint8_t)length;
    token.line = lexer->line;
    token.symbol = lexer->token_start_symbol;

    return token;
}

static Token advanceAndMakeToken(Lexer* lexer, TokenType type) {
    ASSERT_LEXER(lexer);

    advance(lexer);
    return makeToken(lexer, type);
}

static Token makeErrorToken(Lexer* lexer, const char* message) {
    ASSERT_LEXER(lexer);

    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    size_t length = strlen(message);
    assert(length < 256);
    token.length = (uint8_t)length;
    token.line = lexer->line;
    token.symbol = lexer->token_start_symbol;

    if (!isAtEnd(lexer)) {
        advance(lexer);
    }

    return token;
}

static void fillErrorToken(Lexer* lexer, Token* error_token, const char* message) {
    ASSERT_LEXER(lexer);

    error_token->type = TOKEN_ERROR;
    error_token->start = message;
    size_t length = strlen(message);
    assert(length < 256);
    error_token->length = (uint8_t)length;
    error_token->line = lexer->line;
    error_token->symbol = lexer->token_start_symbol;

    if (!isAtEnd(lexer)) {
        advance(lexer);
    }
}

static bool isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAlphaNumeric(char c) {
    return isLetter(c) || isDigit(c) || c == '-';
}

static bool skipWhitespaceAndComments(Lexer* lexer, Token* error_token) {
    ASSERT_LEXER(lexer);

    while (true) {
        switch (current(lexer)) {
            
            // Whitespace
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                advance(lexer);
                break;

            // Single-line comment
            case '|':
                do {
                    advance(lexer);
                } while (!isAtEnd(lexer) && current(lexer) != '\n');
                break;
            
            // Multiline comment
            case '/':
                if (next(lexer) == '-') {
                    // Consume the opening '/-'
                    advance(lexer);
                    advance(lexer);

                    // Match everything prior to '-/' or EOF.
                    do {
                        advance(lexer);
                    } while (!isAtEnd(lexer) && !(current(lexer) == '-' && next(lexer) == '/'));

                    // If it's an unterminated multiline comment, fill error token.
                    if (isAtEnd(lexer)) {
                        fillErrorToken(lexer, error_token, "Unterminated multiline comment.");
                        ASSERT_LEXER(lexer);
                        return false;
                    }

                    // Advance the enclosing '-/'.
                    else {
                        assert(match(lexer, '-'));
                        assert(match(lexer, '/'));
                    }

                    break;
                }
                
                // Not a multiline comment.
                else {
                    ASSERT_LEXER(lexer);
                    return true;
                }

            // Skipped all whitespace and comments
            default:
                ASSERT_LEXER(lexer);
                return true;
        }
    }
}

static Token string(Lexer* lexer) {
    ASSERT_LEXER(lexer);

    // Match everything prior to enclosing '
    do {
        // Unterminated string
        if (isAtEnd(lexer)) {
            ASSERT_LEXER(lexer);
            return makeErrorToken(lexer, "Unterminated string.");
        }

        // Multiline string
        if (current(lexer) == '\n') {
            ASSERT_LEXER(lexer); 
            return makeErrorToken(lexer, "Multiline strings are not allowed.");
        }

        advance(lexer);
    } while (current(lexer) != '\'');

    // Match the enclosing '
    assert(match(lexer, '\''));

    ASSERT_LEXER(lexer);
    return makeToken(lexer, TOKEN_STRING_VALUE);
}

static Token identifier(Lexer* lexer) {
    ASSERT_LEXER(lexer);
    assert(isLetter(current(lexer)));

    char c;
    do {
        advance(lexer);
    } while (isAlphaNumeric(c = current(lexer)));

    if (c == '_') {
        ASSERT_LEXER(lexer);
        return makeErrorToken(lexer, "Identifier can't include '_'. Use '-'.");
    }

    if (c == '-') {
        ASSERT_LEXER(lexer);
        return makeErrorToken(lexer, "An identifier can't end with a '-'.");
    }

    ASSERT_LEXER(lexer);
    return makeToken(lexer, identifierTokenType(lexer));
}

static TokenType identifierTokenType(const Lexer* lexer) {
    ASSERT_LEXER(lexer);

    size_t length = (size_t)(lexer->current - lexer->token_start);

#define TRY_MATCH_TWO_KEYWORDS(index,                                                \
    case_1, keyword_1, token_type_1,                                                 \
    case_2, keyword_2, token_type_2                                                  \
)                                                                                    \
    {                                                                                \
        if (length > index) {                                                        \
            switch (lexer->token_start[index]) {                                     \
                case case_1: return tryMatchKeyword(lexer, keyword_1, token_type_1); \
                case case_2: return tryMatchKeyword(lexer, keyword_2, token_type_2); \
            }                                                                        \
        }                                                                            \
        return TOKEN_IDENTIFIER;                                                     \
    }

#define TRY_MATCH_FOUR_KEYWORDS(index,                                               \
    case_1, keyword_1, token_type_1,                                                 \
    case_2, keyword_2, token_type_2,                                                 \
    case_3, keyword_3, token_type_3,                                                 \
    case_4, keyword_4, token_type_4                                                  \
)                                                                                    \
    {                                                                                \
        if (length > index) {                                                        \
            switch (lexer->token_start[index]) {                                     \
                case case_1: return tryMatchKeyword(lexer, keyword_1, token_type_1); \
                case case_2: return tryMatchKeyword(lexer, keyword_2, token_type_2); \
                case case_3: return tryMatchKeyword(lexer, keyword_3, token_type_3); \
                case case_4: return tryMatchKeyword(lexer, keyword_4, token_type_4); \
            }                                                                        \
        }                                                                            \
        return TOKEN_IDENTIFIER;                                                     \
    }

    switch (lexer->token_start[0]) {
        case 'c': return tryMatchKeyword(lexer, "continue",  TOKEN_CONTINUE);
        case 'd': return tryMatchKeyword(lexer, "do",        TOKEN_DO);
        case 'm': return tryMatchKeyword(lexer, "mutable",   TOKEN_MUTABLE);
        case 'o': return tryMatchKeyword(lexer, "or",        TOKEN_OR);
        case 'r': return tryMatchKeyword(lexer, "return",    TOKEN_RETURN);
        case 't': return tryMatchKeyword(lexer, "true",      TOKEN_TRUE);
        case 'v': return tryMatchKeyword(lexer, "var",       TOKEN_VAR);
        case 'w': return tryMatchKeyword(lexer, "while",     TOKEN_WHILE);

        case 'a': TRY_MATCH_TWO_KEYWORDS( 1, 'n', "and",       TOKEN_AND,
                                             's', "assert",    TOKEN_ASSERT);
        case 'b': TRY_MATCH_TWO_KEYWORDS( 1, 'o', "bool",      TOKEN_BOOL,
                                             'r', "break",     TOKEN_BREAK);
        case 'e': TRY_MATCH_TWO_KEYWORDS( 1, 'l', "else",      TOKEN_ELSE,
                                             'n', "enum",      TOKEN_ENUM);
        case 'f': TRY_MATCH_FOUR_KEYWORDS(1, 'a', "false",     TOKEN_FALSE,
                                             'l', "float",     TOKEN_FLOAT,
                                             'o', "for",       TOKEN_FOR,
                                             'u', "function",  TOKEN_FUNCTION);
        case 'p': TRY_MATCH_TWO_KEYWORDS( 2, 'e', "predicate", TOKEN_PREDICATE,
                                             'i', "print",     TOKEN_PRINT);
        case 's': TRY_MATCH_TWO_KEYWORDS( 3, 'i', "string",    TOKEN_STRING,
                                             'u', "structure", TOKEN_STRUCTURE);

        case 'i':
            if (length == 2) {
                switch (lexer->token_start[1]) {
                    case 'f': return TOKEN_IF;
                    case 'n': return TOKEN_IN;
                }
            } else if (length == 3) {
                return tryMatchKeyword(lexer, "int", TOKEN_INT);
            }
            break;
    }

#undef TRY_MATCH_FOUR_KEYWORDS
#undef TRY_MATCH_TWO_KEYWORDS

    return TOKEN_IDENTIFIER;
}

static TokenType tryMatchKeyword(const Lexer* lexer, const char* keyword, TokenType keywordTokenType) {
    ASSERT_LEXER(lexer);

    size_t length = (size_t)(lexer->current - lexer->token_start);

    if (length == strlen(keyword) && memcmp(lexer->token_start, keyword, length) == 0) {
        return keywordTokenType;
    } else {
        return TOKEN_IDENTIFIER;
    }
}

static Token number(Lexer* lexer) {
    ASSERT_LEXER(lexer);
    assert(isDigit(current(lexer)));

    TokenType token_type = TOKEN_INTEGER_VALUE;

    // Match integer part
    if (match(lexer, 0)) {
        // Do nothing
    } else {
        while (isDigit(current(lexer))) {
            advance(lexer);
        }
    }

    // Match floating point part
    // At least one digit after floating point
    if (match(lexer, '.') && isDigit(current(lexer))) {
        do {
            advance(lexer);
        } while (isDigit(current(lexer)));
        token_type = TOKEN_FLOAT_VALUE;
    }

    // TODO: check number length.

    ASSERT_LEXER(lexer);
    return makeToken(lexer, token_type);
}


#undef ASSERT_LEXER

