#include "cut.h"

#include "lexer.h"
#include "token.h"


#define EXPECT_TOKEN_TYPE_SEQUENCE(lexer, ...)                 \
    {                                                          \
        TokenType token_types[] = { __VA_ARGS__ };             \
        size_t n = sizeof token_types / sizeof token_types[0]; \
        for (size_t i = 0; i < n; ++i) {                       \
            EXPECT_EQUALS_TOSTR(                               \
                readToken(&lexer).type,                        \
                token_types[i],                                \
                tokenTypeName                                  \
            );                                                 \
        }                                                      \
        EXPECT_EQUALS_TOSTR(                                   \
            readToken(&lexer).type,                            \
            TOKEN_END,                                         \
            tokenTypeName                                      \
        );                                                     \
    }

#define TEST_LEXER(name, input, ...)                    \
    TEST(name) {                                        \
        Lexer lexer;                                    \
        initLexer(&lexer, input);                       \
        EXPECT_TOKEN_TYPE_SEQUENCE(lexer, __VA_ARGS__); \
        freeLexer(&lexer);                              \
    } static_assert(true, "require semicolon")


TEST_LEXER(SimpleTokensWithWhitespace,
    ": , . { [ ( } ] ) ;",
    TOKEN_COLON,    TOKEN_COMMA,    TOKEN_DOT,
    TOKEN_LBRACE,   TOKEN_LBRACKET, TOKEN_LPAREN,
    TOKEN_RBRACE,   TOKEN_RBRACKET, TOKEN_RPAREN,
    TOKEN_SEMICOLON
);

TEST_LEXER(SimpleTokensWithoutWhitespace,
    ":,.{[(}]);",
    TOKEN_COLON,    TOKEN_COMMA,    TOKEN_DOT,
    TOKEN_LBRACE,   TOKEN_LBRACKET, TOKEN_LPAREN,
    TOKEN_RBRACE,   TOKEN_RBRACKET, TOKEN_RPAREN,
    TOKEN_SEMICOLON
);

TEST_LEXER(TokenOrTokenEqualsWithWhitespace,
    "= == ! != > >= < <= - -= % %= + += / /= * *=",
    TOKEN_EQUAL,        TOKEN_EQUAL_EQUAL,
    TOKEN_EXCLAMATION,  TOKEN_EXCLAMATION_EQUAL,
    TOKEN_GREATER,      TOKEN_GREATER_EQUAL,
    TOKEN_LESS,         TOKEN_LESS_EQUAL,
    TOKEN_MINUS,        TOKEN_MINUS_EQUAL,
    TOKEN_PERCENT,      TOKEN_PERCENT_EQUAL,
    TOKEN_PLUS,         TOKEN_PLUS_EQUAL,
    TOKEN_SLASH,        TOKEN_SLASH_EQUAL,
    TOKEN_STAR,         TOKEN_STAR_EQUAL
);

TEST_LEXER(TokenOrTokenEqualsWithoutWhitespace,
    "= ==!!=>>=<<=--=%%=++=//=**=",
    TOKEN_EQUAL,        TOKEN_EQUAL_EQUAL,
    TOKEN_EXCLAMATION,  TOKEN_EXCLAMATION_EQUAL,
    TOKEN_GREATER,      TOKEN_GREATER_EQUAL,
    TOKEN_LESS,         TOKEN_LESS_EQUAL,
    TOKEN_MINUS,        TOKEN_MINUS_EQUAL,
    TOKEN_PERCENT,      TOKEN_PERCENT_EQUAL,
    TOKEN_PLUS,         TOKEN_PLUS_EQUAL,
    TOKEN_SLASH,        TOKEN_SLASH_EQUAL,
    TOKEN_STAR,         TOKEN_STAR_EQUAL
);

TEST_LEXER(Comments,
    "+ | comment\n- //- multiline \n comment -/ > /- unterminated comment",
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_SLASH, TOKEN_GREATER, TOKEN_ERROR
);

TEST_LEXER(Identifiers,
    "and        an         assert   break    \n"
    "continue   else       elses    enum     \n"
    "false      forfalse   for      function \n"
    "if         in         mutable  is       \n"
    "or         predicate  print    return   \n"
    "structure  true       var      while    \n",
    TOKEN_AND,       TOKEN_IDENTIFIER, TOKEN_ASSERT,     TOKEN_BREAK,
    TOKEN_CONTINUE,  TOKEN_ELSE,       TOKEN_IDENTIFIER, TOKEN_ENUM,
    TOKEN_FALSE,     TOKEN_IDENTIFIER, TOKEN_FOR,        TOKEN_FUNCTION,
    TOKEN_IF,        TOKEN_IN,         TOKEN_MUTABLE,    TOKEN_IDENTIFIER,
    TOKEN_OR,        TOKEN_PREDICATE,  TOKEN_PRINT,      TOKEN_RETURN,
    TOKEN_STRUCTURE, TOKEN_TRUE,       TOKEN_VAR,        TOKEN_WHILE
);

TEST_LEXER(Numbers,
    "0 0.1",
    TOKEN_INTEGER_VALUE, TOKEN_FLOAT_VALUE
);

// TEST(STRING) {
//     // TODO
// }


#undef TEST_LEXER
#undef EXPECT_TOKEN_TYPE_SEQUENCE

