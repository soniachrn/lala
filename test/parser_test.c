#include "cut.h"

#include "parser.h"


#define EXPECT_BINARY_SEQUENCE(chunk, ...)                  \
    {                                                       \
        uint8_t expected[] = { __VA_ARGS__ };               \
        size_t n = (size_t)(chunk.stack_top - chunk.stack); \
        for (size_t i = 0; i < n; ++i) {                    \
            EXPECT_EQUALS_F(                                \
                chunk.stack[i],                             \
                expected[i],                                \
                "0x%02X"                                    \
            );                                              \
        }                                                   \
    }

#define TEST_PARSER(name, input, ...)               \
    TEST(name) {                                    \
        Lexer lexer;                                \
        initLexer(&lexer, input);                   \
                                                    \
        Stack chunk;                                \
        initStack(&chunk);                          \
                                                    \
        Parser parser;                              \
        initParser(&parser, &lexer, &chunk);        \
                                                    \
        parseExpression(&parser);                   \
                                                    \
        EXPECT_BINARY_SEQUENCE(chunk, __VA_ARGS__); \
                                                    \
        freeParser(&parser);                        \
        freeStack(&chunk);                          \
        freeLexer(&lexer);                          \
    } static_assert(true, "require semicolon")

#define BINARY_FLOAT_0_5 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x3F
#define BINARY_FLOAT_2   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40
#define BINARY_FLOAT_4   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x40

#define TEST_PARSER_INT_OPERATOR(name, operator_str, ...)   \
    TEST_PARSER(name,                                       \
        "16 " operator_str " 4",                            \
        OP_PUSH_INT, 0x10, 0x00, 0x00, 0x00,                \
        OP_PUSH_INT, 0x04, 0x00, 0x00, 0x00,                \
        __VA_ARGS__                                         \
    )

#define TEST_PARSER_FLOAT_OPERATOR(name, operator_str, ...) \
    TEST_PARSER(name,                                       \
        "2.0 " operator_str " 0.5",                         \
        OP_PUSH_FLOAT, BINARY_FLOAT_2,                      \
        OP_PUSH_FLOAT, BINARY_FLOAT_0_5,                    \
        __VA_ARGS__                                         \
    )


TEST_PARSER(PushTrue,
    "true",
    OP_PUSH_TRUE
);

TEST_PARSER(PushFalse,
    "false",
    OP_PUSH_FALSE
);

TEST_PARSER(PushInt,
    "5",
    OP_PUSH_INT, 0x05, 0x00, 0x00, 0x00  // 5
);

TEST_PARSER(PushFloat,
    "0.5",
    OP_PUSH_FLOAT, BINARY_FLOAT_0_5,
);

TEST_PARSER(NegateBool,
    "!false",
    OP_PUSH_FALSE,
    OP_NEGATE_BOOL
);

TEST_PARSER(NegateInt,
    "-12",
    OP_PUSH_INT,  0x0C, 0x00, 0x00, 0x00,  // 12
    OP_NEGATE_INT
);

TEST_PARSER(NegateFloat,
    "-0.5",
    OP_PUSH_FLOAT,  BINARY_FLOAT_0_5,
    OP_NEGATE_FLOAT
);

TEST_PARSER_INT_OPERATOR(  MultiplyInt,          "*",  OP_MULTIPLY_INT);
TEST_PARSER_FLOAT_OPERATOR(MultiplyFloat,        "*",  OP_MULTIPLY_FLOAT);

TEST_PARSER_INT_OPERATOR(  DivideInt,            "/",  OP_DIVIDE_INT);
TEST_PARSER_FLOAT_OPERATOR(DivideFloat,          "/",  OP_DIVIDE_FLOAT);

TEST_PARSER_INT_OPERATOR(  ModuloInt,            "%",  OP_MODULO_INT);

TEST_PARSER_INT_OPERATOR(  AddInt,               "+",  OP_ADD_INT);
TEST_PARSER_FLOAT_OPERATOR(AddFloat,             "+",  OP_ADD_FLOAT);

TEST_PARSER_INT_OPERATOR(  SubtractInt,          "-",  OP_NEGATE_INT,    OP_ADD_INT);
TEST_PARSER_FLOAT_OPERATOR(SubtractFloat,        "-",  OP_NEGATE_FLOAT,  OP_ADD_FLOAT);

TEST_PARSER_INT_OPERATOR(  EqualsInt,            "==", OP_EQUALS_INT);
TEST_PARSER_FLOAT_OPERATOR(EqualsFloat,          "==", OP_EQUALS_FLOAT);

TEST_PARSER_INT_OPERATOR(  NotEqualsInt,         "!=", OP_EQUALS_INT,    OP_NEGATE_BOOL);
TEST_PARSER_FLOAT_OPERATOR(NotEqualsFloat,       "!=", OP_EQUALS_FLOAT,  OP_NEGATE_BOOL);

TEST_PARSER_INT_OPERATOR(  GreaterInt,           ">",  OP_GREATER_INT);
TEST_PARSER_FLOAT_OPERATOR(GreaterFloat,         ">",  OP_GREATER_FLOAT);

TEST_PARSER_INT_OPERATOR(  LessInt,              "<",  OP_LESS_INT);
TEST_PARSER_FLOAT_OPERATOR(LessFloat,            "<",  OP_LESS_FLOAT);

TEST_PARSER_INT_OPERATOR(  GreaterOrEqualsInt,   ">=", OP_LESS_INT,      OP_NEGATE_BOOL);
TEST_PARSER_FLOAT_OPERATOR(GreaterOrEqualsFloat, ">=", OP_LESS_FLOAT,    OP_NEGATE_BOOL);

TEST_PARSER_INT_OPERATOR(  LessOrEqualsInt,      "<=", OP_GREATER_INT,   OP_NEGATE_BOOL);
TEST_PARSER_FLOAT_OPERATOR(LessOrEqualsFloat,    "<=", OP_GREATER_FLOAT, OP_NEGATE_BOOL);

TEST_PARSER(TermFactorPrecedence,
    "2 * 3 + 4",
    OP_PUSH_INT,     0x02, 0x00, 0x00, 0x00, // 2
    OP_PUSH_INT,     0x03, 0x00, 0x00, 0x00, // 3
    OP_MULTIPLY_INT,
    OP_PUSH_INT,     0x04, 0x00, 0x00, 0x00, // 4
    OP_ADD_INT
);

TEST_PARSER(FactorTermPrecedence,
    "2 - 3 / 4",
    OP_PUSH_INT,   0x02, 0x00, 0x00, 0x00, // 2
    OP_PUSH_INT,   0x03, 0x00, 0x00, 0x00, // 3
    OP_PUSH_INT,   0x04, 0x00, 0x00, 0x00, // 4
    OP_DIVIDE_INT,
    OP_NEGATE_INT,
    OP_ADD_INT,
);

TEST_PARSER(ComplexExpression,
    "4 / (1 + 3) > 5 and 2.0 != 4.0 * 0.5 and true",
    OP_PUSH_INT,       0x04, 0x00, 0x00, 0x00, // 4
    OP_PUSH_INT,       0x01, 0x00, 0x00, 0x00, // 1
    OP_PUSH_INT,       0x03, 0x00, 0x00, 0x00, // 3
    OP_ADD_INT,
    OP_DIVIDE_INT,    
    OP_PUSH_INT,       0x05, 0x00, 0x00, 0x00, // 5
    OP_GREATER_INT,
    OP_PUSH_FLOAT,     BINARY_FLOAT_2,
    OP_PUSH_FLOAT,     BINARY_FLOAT_4,
    OP_PUSH_FLOAT,     BINARY_FLOAT_0_5,
    OP_MULTIPLY_FLOAT,
    OP_EQUALS_FLOAT,
    OP_NEGATE_BOOL,
    OP_AND,
    OP_PUSH_TRUE,
    OP_AND
);

#undef TEST_PARSER_FLOAT_OPERATOR
#undef TEST_PARSER_INT_OPERATOR

#undef BINARY_FLOAT_4
#undef BINARY_FLOAT_2
#undef BINARY_FLOAT_0_5

#undef TEST_PARSER
#undef EXPECT_BINARY_SEQUENCE

