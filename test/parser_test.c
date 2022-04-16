#include "cut.h"

#include "parser.h"


static Parser* createParser(const char* input) {
    Lexer* lexer = calloc(1, sizeof(Lexer));
    initLexer(lexer, input);
    
    Stack* chunk = calloc(1, sizeof(Stack));
    initStack(chunk);

    Parser* parser = calloc(1, sizeof(Parser));
    initParser(parser, lexer, chunk);

    return parser;
}

static void deleteParser(Parser* parser) {
    Stack* chunk = parser->chunk;
    Lexer* lexer = parser->lexer;

    freeParser(parser);
    freeStack(chunk);
    freeLexer(lexer);

    free(parser);
    free(chunk);
    free(lexer);
}

#define EXPECT_BINARY_SEQUENCE(chunk, ...)                  \
    {                                                       \
        uint8_t expected[] = { __VA_ARGS__ };               \
        size_t n = (size_t)(chunk->stack_top - chunk->stack); \
        for (size_t i = 0; i < n; ++i) {                    \
            EXPECT_EQUALS_F(                                \
                chunk->stack[i],                             \
                expected[i],                                \
                "0x%02X"                                    \
            );                                              \
        }                                                   \
    }

#define TEST_PARSER_EXPRESSION(name, input, ...)            \
    TEST(name) {                                            \
        Parser* parser = createParser(input);               \
        parseExpression(parser);                            \
        EXPECT_BINARY_SEQUENCE(parser->chunk, __VA_ARGS__); \
                                                            \
        deleteParser(parser);                               \
    } static_assert(true, "require semicolon")

#define BINARY_FLOAT_0_5 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x3F
#define BINARY_FLOAT_2   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40
#define BINARY_FLOAT_4   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x40


TEST_PARSER_EXPRESSION(PushTrue,
    "true",
    OP_PUSH_TRUE
);

TEST_PARSER_EXPRESSION(PushFalse,
    "false",
    OP_PUSH_FALSE
);

TEST_PARSER_EXPRESSION(PushInt,
    "5",
    OP_PUSH_INT, 0x05, 0x00, 0x00, 0x00  // 5
);

TEST_PARSER_EXPRESSION(PushFloat,
    "0.5",
    OP_PUSH_FLOAT, BINARY_FLOAT_0_5,
);

TEST_PARSER_EXPRESSION(NegateBool,
    "!false",
    OP_PUSH_FALSE,
    OP_NEGATE_BOOL
);

TEST_PARSER_EXPRESSION(NegateInt,
    "-12",
    OP_PUSH_INT,  0x0C, 0x00, 0x00, 0x00,  // 12
    OP_NEGATE_INT
);

TEST_PARSER_EXPRESSION(NegateFloat,
    "-0.5",
    OP_PUSH_FLOAT,  BINARY_FLOAT_0_5,
    OP_NEGATE_FLOAT
);

#define TEST_PARSER_INT_OPERATOR(name, operator_str, ...) \
    TEST_PARSER_EXPRESSION(name,                          \
        "16 " operator_str " 4",                          \
        OP_PUSH_INT, 0x10, 0x00, 0x00, 0x00,              \
        OP_PUSH_INT, 0x04, 0x00, 0x00, 0x00,              \
        __VA_ARGS__                                       \
    )

#define TEST_PARSER_FLOAT_OPERATOR(name, operator_str, ...) \
    TEST_PARSER_EXPRESSION(name,                            \
        "2.0 " operator_str " 0.5",                         \
        OP_PUSH_FLOAT, BINARY_FLOAT_2,                      \
        OP_PUSH_FLOAT, BINARY_FLOAT_0_5,                    \
        __VA_ARGS__                                         \
    )

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

#undef TEST_PARSER_FLOAT_OPERATOR
#undef TEST_PARSER_INT_OPERATOR

TEST_PARSER_EXPRESSION(TermFactorPrecedence,
    "2 * 3 + 4",
    OP_PUSH_INT,     0x02, 0x00, 0x00, 0x00, // 2
    OP_PUSH_INT,     0x03, 0x00, 0x00, 0x00, // 3
    OP_MULTIPLY_INT,
    OP_PUSH_INT,     0x04, 0x00, 0x00, 0x00, // 4
    OP_ADD_INT
);

TEST_PARSER_EXPRESSION(FactorTermPrecedence,
    "2 - 3 / 4",
    OP_PUSH_INT,   0x02, 0x00, 0x00, 0x00, // 2
    OP_PUSH_INT,   0x03, 0x00, 0x00, 0x00, // 3
    OP_PUSH_INT,   0x04, 0x00, 0x00, 0x00, // 4
    OP_DIVIDE_INT,
    OP_NEGATE_INT,
    OP_ADD_INT,
);

TEST_PARSER_EXPRESSION(ComplexExpression,
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

#define EXPECT_VARIABLE(scope, name, value_type, address)                    \
    {                                                                        \
        Variable variable;                                                   \
        EXPECT(accessVariableInScope(scope, name, strlen(name), &variable)); \
        EXPECT_EQUALS_TOSTR(variable.type, value_type, valueTypeName);       \
        EXPECT_EQUALS(variable.address_on_stack, address);                   \
    }

TEST(DefaultInitializedVariables) {
    Parser* parser = createParser(
        "var boolean: bool\n"
        "var integer: int\n"
        "var floating-point: float\n"
    );

    parse(parser);

    EXPECT_BINARY_SEQUENCE(
        parser->chunk,
        OP_PUSH_FALSE,
        OP_PUSH_INT,   0x00, 0x00, 0x00, 0x00,
        OP_PUSH_FLOAT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    );

    EXPECT_VARIABLE(parser->scope, "boolean",        VALUE_BOOL,  0);
    EXPECT_VARIABLE(parser->scope, "integer",        VALUE_INT,   1);
    EXPECT_VARIABLE(parser->scope, "floating-point", VALUE_FLOAT, 5);

    deleteParser(parser);
}

TEST(ExplicitlyInitializedVariables) {
    Parser* parser = createParser(
        "var boolean: bool = true\n"
        "var integer: int  = 4\n"
        "var floating-point: float = 0.5\n"
    );

    parse(parser);

    EXPECT_BINARY_SEQUENCE(
        parser->chunk,
        OP_PUSH_TRUE,
        OP_PUSH_INT,   0x04, 0x00, 0x00, 0x00,  // 4
        OP_PUSH_FLOAT, BINARY_FLOAT_0_5  // 0.5
    );

    EXPECT_VARIABLE(parser->scope, "boolean",        VALUE_BOOL,  0);
    EXPECT_VARIABLE(parser->scope, "integer",        VALUE_INT,   1);
    EXPECT_VARIABLE(parser->scope, "floating-point", VALUE_FLOAT, 5);

    deleteParser(parser);
}

TEST(AccessAndPrintVariable) {
    Parser* parser = createParser(
        "var boolean: bool = true\n"
        "var integer: int  = 4\n"
        "var floating-point: float = 0.5\n"
        "print boolean\n"
        "print integer\n"
        "print floating-point\n"
    );

    parse(parser);

    EXPECT_BINARY_SEQUENCE(
        parser->chunk,
        OP_PUSH_TRUE,                           // var boolean: bool = true
        OP_PUSH_INT,   0x04, 0x00, 0x00, 0x00,  // var integer: int  = 4
        OP_PUSH_FLOAT, BINARY_FLOAT_0_5,        // var floating-point: float = 0.5
        // print boolean
        OP_GET_BYTE_FROM_STACK,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 0
        OP_PRINT_BOOL,
        // print integer
        OP_GET_INT_FROM_STACK,   0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 1
        OP_PRINT_INT,
        // print floating-point
        OP_GET_FLOAT_FROM_STACK, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 5
        OP_PRINT_FLOAT
    );

    EXPECT_VARIABLE(parser->scope, "boolean",        VALUE_BOOL,  0);
    EXPECT_VARIABLE(parser->scope, "integer",        VALUE_INT,   1);
    EXPECT_VARIABLE(parser->scope, "floating-point", VALUE_FLOAT, 5);

    deleteParser(parser);
}

TEST(AssignVariable) {
    Parser* parser = createParser(
        "var b1: bool = true\n"
        "var b2: bool = false\n"
        "var i1: int  = 4\n"
        "var i2: int  = 8\n"
        "var f1: float = 0.5\n"
        "var f2: float = 2.0\n"
        "b1 = b2\n"
        "i1 = i2\n"
        "f1 = f2\n"
    );

    parse(parser);

    EXPECT_BINARY_SEQUENCE(
        parser->chunk,
        OP_PUSH_TRUE,                           // var b1: bool  = true
        OP_PUSH_FALSE,                          // var b2: bool  = false
        OP_PUSH_INT,   0x04, 0x00, 0x00, 0x00,  // var i1: int   = 4
        OP_PUSH_INT,   0x08, 0x00, 0x00, 0x00,  // var i2: int   = 8
        OP_PUSH_FLOAT, BINARY_FLOAT_0_5,        // var f1: float = 0.5
        OP_PUSH_FLOAT, BINARY_FLOAT_2,          // var f2: float = 2
        // b1 = b2
        OP_GET_BYTE_FROM_STACK,  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 1 (b2)
        OP_SET_BYTE_ON_STACK,    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 0 (b1)
        // i1 = i2
        OP_GET_INT_FROM_STACK,   0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 6 (i2)
        OP_SET_INT_ON_STACK,     0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 2 (i1)
        // f1 = f2
        OP_GET_FLOAT_FROM_STACK, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 18 (f2)
        OP_SET_FLOAT_ON_STACK,   0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 10 (f1)
    );

    EXPECT_VARIABLE(parser->scope, "b1", VALUE_BOOL,  0);
    EXPECT_VARIABLE(parser->scope, "b2", VALUE_BOOL,  1);
    EXPECT_VARIABLE(parser->scope, "i1", VALUE_INT,   2);
    EXPECT_VARIABLE(parser->scope, "i2", VALUE_INT,   6);
    EXPECT_VARIABLE(parser->scope, "f1", VALUE_FLOAT, 10);
    EXPECT_VARIABLE(parser->scope, "f2", VALUE_FLOAT, 18);

    deleteParser(parser);
}


#undef BINARY_FLOAT_4
#undef BINARY_FLOAT_2
#undef BINARY_FLOAT_0_5

#undef TEST_PARSER
#undef EXPECT_BINARY_SEQUENCE

