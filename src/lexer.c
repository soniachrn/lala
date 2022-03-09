// ❤️

#include "lexer.h"


// ┌────────┐
// │ Macros │
// └────────┘

#define ASSERT_LEXER(lexer) \
    { \
        assert(lexer); \
        assert(lexer->token_start); \
        assert(lexer->current); \
        assert(lexer->line >= 1); \
        assert(lexer->symbol >= 1); \
    }

#define ASSERT_UNINITIALIZED_LEXER(lexer) \
    { \
        assert(lexer); \
        assert(!lexer->token_start); \
        assert(!lexer->current); \
        assert(lexer->line == 0); \
        assert(lexer->token_start_symbol == 0); \
        assert(lexer->symbol == 0); \
    }

// ┌──────────────────────────────┐
// │ Static function declarations │
// └──────────────────────────────┘

static char current(const Lexer& lexer);
static bool isAtEnd(const Lexer& lexer);
static void advance(Lexer& lexer);
static bool match(Lexer& lexer, char c);

static void startNewToken(Lexer& lexer);
static Token makeToken(const Lexer& lexer, TokenType type);
static Token advanceAndMakeToken(Lexer& lexer, TokenType type);
static Token makeErrorToken(const Lexer& lexer, const char* message);

static bool isLetter(char c);
static bool isDigit(char c);
static bool isAlphaNumeric(char c);     // [-a-zA-Z0-9]

static void skipWhitespaceAndComments(Lexer& lexer);
static Token string(Lexer& lexer);
static Token identifier(Lexer& lexer);
static TokenType identifierTokenType(Lexer& lexer);
static Token number(Lexer& lexer);


// ┌──────────────────────────┐
// │ Function implementations │
// └──────────────────────────┘

void initLexer(Lexer& lexer, const char* input) {
    ASSERT_UNINITIALIZED_LEXER(lexer);

    lexer->token_start = input;
    lexer->current = input;
    lexer->line = 1;
    lexer->token_start_symbol = 1;
    lexer->symbol = 1;

    ASSERT_LEXER(lexer);
}

void freeLexer(Lexer& lexer) {
    ASSERT_LEXER(lexer);

    lexer->token_start = NULL;
    lexer->current = NULL;
    lexer->line = 0;
    lexer->token_start_symbol = 0;
    lexer->symbol = 0;

    ASSERT_UNINITIALIZED_LEXER(lexer);
}

void dumpLexer(const Lexer& lexer, FILE* out) {
    assert(out);

    if (!lexer) {
        fprintf(out, "Lexer *(NULL)\n")
    } else {
        fprintf(out, "Lexer *(%p) {\n", lexer);
        fprintf(out, "  token_start        = *(%p) '%c'\n", lexer->token_start, lexer->token_start);
        fprintf(out, "  current            = *(%p) '%c'\n", lexer->current, lexer->current);
        fprintf(out, "  line               = %d\n", lexer->line);
        fprintf(out, "  token_start_symbol = %d\n", lexer->token_start_symbol);
        fprintf(out, "  symbol             = %d\n", lexer->symbol);
        fprintf(out, "}\n");
    }
}

const Token readToken(Lexer& lexer) {
    ASSERT_LEXER(lexer);

    skipWhitespaceAndComments(lexer);
    startNewToken(lexer);

#define TOKEN_OR_TOKEN_EQUAL(tokenEqualType, tokenType) \
    advanceAndMakeToken(lexer, match(lexer, '=') ? tokenEqualType : tokenType)

    char c = current(lexer);
    switch (c) {

        // One symbol
        case ':': return advanceAndMakeToken(lexer, TOKEN_COLON);
        case ',': return advanceAndMakeToken(lexer, TOKEN_COMMA);
        case '.': return advanceAndMakeToken(lexer, TOKEN_DOT);
        case '{': return advanceAndMakeToken(lexer, TOKEN_LBRACE);
        case '[': return advanceAndMakeToken(lexer, TOKEN_LBRACKET);
        case '(': return advanceAndMakeToken(lexer, TOKEN_LPAREN);
        case '?': return advanceAndMakeToken(lexer, TOKEN_QUESTION);
        case '}': return advanceAndMakeToken(lexer, TOKEN_RBRACE);
        case ']': return advanceAndMakeToken(lexer, TOKEN_RBRACKET);
        case ')': return advanceAndMakeToken(lexer, TOKEN_RPAREN);

        // One or two symbols
        case '=': return TOKEN_OR_TOKEN_EQUAL(TOKEN_EQUAL_EQUAL,       TOKEN_EQUAL);
        case '!': return TOKEN_OR_TOKEN_EQUAL(TOKEN_EXCLAMATION_EQUAL, TOKEN_EXCLAMATION);
        case '>': return TOKEN_OR_TOKEN_EQUAL(TOKEN_GREATER_EQUAL,     TOKEN_GREATER);
        case '<': return TOKEN_OR_TOKEN_EQUAL(TOKEN_LESS_EQUAL,        TOKEN_LESS);
        case '-': return TOKEN_OR_TOKEN_EQUAL(TOKEN_MINUS_EQUAL,       TOKEN_MINUS);
        case '%': return TOKEN_OR_TOKEN_EQUAL(TOKEN_PERCENT_EQUAL,     TOKEN_PERCENT);
        case '+': return TOKEN_OR_TOKEN_EQUAL(TOKEN_PLUS_EQUAL,        TOKEN_PLUS);
        case '/': return TOKEN_OR_TOKEN_EQUAL(TOKEN_SLASH_EQUAL,       TOKEN_SLASH);
        case '*': return TOKEN_OR_TOKEN_EQUAL(TOKEN_STAR_EQUAL,        TOKEN_STAR);

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

static char current(const Lexer& lexer) {
    ASSERT_LEXER(lexer);
    return lexer->current[0];
}

static bool isAtEnd(const Lexer& lexer) {
    ASSERT_LEXER(lexer);
    return current(lexer) == '\0';
}

static void advance(Lexer& lexer) {
    ASSERT_LEXER(lexer);
    assert(!isAtEnd(lexer));

    if (current(lexer) == '\n') {
        lexer->line += 1;
        lexer->symbol = 1;
    } else {
        lexer->symbol += 1;
    }
    lexer->current++;

    ASSERT_LEXER(lexer);
}

static bool match(Lexer& lexer, char expected) {
    ASSERT_LEXER(lexer);

    if (current(lexer) == expected) {
        advance(lexer);
        return true;
    } else {
        return false;
    }
}

static void startNewToken(Lexer& lexer) {
    ASSERT_LEXER(lexer);

    lexer->token_start = lexer->current;
    lexer->token_start_symbol = lexer->symbol;

    ASSERT_LEXER(lexer);
}

static Token makeToken(const Lexer& lexer, TokenType type) {
    ASSERT_LEXER(lexer);

    Token token;
    token->type = type;
    token->start = lexer->token_start;
    token->length = lexer->current - lexer->token_start;
    token->line = lexer->line;
    token->symbol = lexer->token_start_symbol;

    return token;
}

static Token advanceAndMakeToken(Lexer& lexer, TokenType type) {
    ASSERT_LEXER(lexer);

    advance(lexer);
    return makeToken(lexer, type);
}

static Token makeErrorToken(const Lexer& lexer, const char* message) {
    ASSERT_LEXER(lexer);

    Token token;
    token->type = TOKEN_ERROR;
    token->start = message;
    token->length = strlen(message);
    token->line = lexer->line;
    token->symbol = lexer->token_start_symbol;

    return token;
}

static bool isLetter(char c) {
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAlphaNumeric(char c) {
    return isLetter(c) or isDigit(c) or c == '-';
}

static void skipWhitespaceAndComments(Lexer& lexer) {
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
                if (match(lexer, '-')) {
                    // Currently behaves as if unterminated multiline comments are allowed.
                    
                    // Match everything prior to '-/' or EOF.
                    do {
                        advance(lexer);
                    } while (!isAtEnd(lexer) && !(current(lexer) == '-' && next(lexer) == '/'));

                    // If it's not an unterminated multiline comment, advance the closing '-/'.
                    if (!isAtEnd(lexer)) {
                        assert(match('-'));
                        assert(match('/'));
                    }

                    break;
                }
                
                // Not a multiline comment.
                else {
                    ASSERT_LEXER(lexer);
                    return;
                }

            // Skipped all whitespace and comments
            default:
                ASSERT_LEXER(lexer);
                return;
        }
    }
}

static Token string(Lexer& lexer) {
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
    assert(match('\''));

    ASSERT_LEXER(lexer);
    return makeToken(lexer, TOKEN_STRING);
}

static Token identifier(Lexer& lexer) {
    ASSERT_LEXER(lexer);
    assert(isLetter(current(lexer)));

    char c;
    do {
        advance(lexer);
    } while (isAlphanumeric(c = current(lexer)));

    if (c == '-') {
        ASSERT_LEXER(lexer);
        return makeErrorToken(lexer, "An identifier can't end with a '-'.");
    }

    ASSERT_LEXER(lexer);
    return makeToken(lexer, identifierTokenType(lexer));
}

static TokenType identifierTokenType(Lexer& lexer) {
    ASSERT_LEXER(lexer);

    size_t length = lexer->current - lexer->token_start;
    switch (lexer->token_start[0]) {
        case 'a':
        case 'b':
        case 'c':
        case 'e':
        case 'f':
        case 'i':
        case 'm':
        case 'o':
        case 'p':
        case 'r':
        case 's':
        case 't':
        case 'v':
        case 'w':
        default:
            return TOKEN_IDENTIFIER;
    }
}

static Token number(Lexer& lexer) {
    ASSERT_LEXER(lexer);
    assert(isDigit(current(lexer)));

    // Match integer part
    if (match(0)) {
        // Do nothing
    } else {
        while (isDigit(current(lexer))) {
            advance(lexer);
        }
    }

    // Match floating point part
    // At least one digit after floating point
    if (match('.') && isDigit(next(lexer))) {
        do {
            advance(lexer);
        } while (isDigit(current(lexer)));
    }

    ASSERT_LEXER(lexer);
    return makeToken(lexer, TOKEN_NUMBER);
}


#undef ASSERT_UNINITIALIZED_LEXER
#undef ASSERT_LEXER

