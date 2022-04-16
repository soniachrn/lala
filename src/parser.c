#include "parser.h"


#include <assert.h>
#include <stdlib.h>

#include "debug.h"


// ┌────────┐
// │ Macros │
// └────────┘

#define VALIDATE_PARSER(parser) \
    (                           \
        parser &&               \
        parser->lexer &&        \
        parser->chunk           \
    )

#define ASSERT_PARSER(parser)                            \
    if (!VALIDATE_PARSER(parser)) {                      \
        fprintf(stderr,                                  \
            "%s:%d, in %s:\nParser assertion failed.\n", \
            __FILENAME__,                                \
            __LINE__,                                    \
            __FUNCTION_NAME__                            \
        );                                               \
        fdumpParser(stderr, parser, 0);                  \
        exit(1);                                         \
    }

#define SYMBOL_TABLE_INVALID_ADDRESS 0


// ┌──────────────────────────────┐
// │ Static function declarations │
// └──────────────────────────────┘


// —————————————————————
//  Parser manipulation
// —————————————————————

static void readNext(Parser* parser);

// Advances and returns the consumed token's type.
static TokenType advance(Parser* parser);

// If the current token's type is token_type, advances and returns
// true. Else, returns false.
static bool match(Parser* parser, TokenType expected);

// If the current token's type is token_type, advances and returns 
// the matched token. Else, results in an error.
static Token forceMatch(Parser* parser, TokenType expected);

// Return the last consumed token.
static Token previous(const Parser* parser);

// Returns the type of the next token without consuming it.
static TokenType peekNext(Parser* parser);


// —————————
//  Parsing
// —————————

// Declaration
static void parseDeclaration(Parser* parser);
static void parseVariable(Parser* parser);

// Statement
static void parseStatement(Parser* parser);
static void parsePrint(Parser* parser);
static void parseAssignment(Parser* parser);

// Expression
static ValueType parseOr        (Parser* parser);
static ValueType parseAnd       (Parser* parser);
static ValueType parseComparison(Parser* parser);
static ValueType parseTerm      (Parser* parser);
static ValueType parseFactor    (Parser* parser);
static ValueType parsePrefix    (Parser* parser);
static ValueType parsePostfix   (Parser* parser);
static ValueType parsePrimary   (Parser* parser);


// —————————
//  Utility
// —————————

static uint8_t addStringConstant(const char* start, uint8_t length) { (void)start; (void)length; return 0; }


// —————————————————————
//  Operator type rules 
// —————————————————————

static bool validateOperandType(
    ValueType operand_type,
    ValueType expected_operand_types[4]
);

static bool validateOperatorTypes(
    TokenType operator,
    ValueType left_operand_type,
    ValueType right_operand_type
);


// ────────────────────
//  Operator to OpCode 
// ────────────────────

static void pushOpCodeOnStack(Stack* stack, OpCode op_code) {
    if (op_code != OP_EMPTY) {
        pushByteOnStack(stack, (uint8_t)op_code);
    }
}

static void emitOpCodesForTokenAndValueTypesCombination(
    Parser* parser,
    uint8_t arity,
    TokenType token_type,
    ValueType value_type
);


// ┌──────────────────────────┐
// │ Function implementations │
// └──────────────────────────┘

void initParser(Parser* parser, Lexer* lexer, Stack* chunk) {
    assert(parser);

    parser->lexer = lexer;
    parser->chunk = chunk;
    parser->did_read_next = false;
    parser->scope = createScope(NULL);

    ASSERT_PARSER(parser);
}

void freeParser(Parser* parser) {
    ASSERT_PARSER(parser);

    parser->lexer = NULL;
    parser->chunk = NULL;
    parser->did_read_next = false;
    deleteScope(parser->scope);
}

void dumpParser(const Parser* parser) {
    fdumpParser(stdout, parser, 0);
}

void fdumpParser(FILE* out, const Parser* parser, int padding) {
    assert(out);

#define printf(...)                                \
    {                                              \
        if (padding > 0) {                         \
            fprintf(out, "%*s", padding * 2, " "); \
        }                                          \
        fprintf(out, __VA_ARGS__);                 \
    }

    if (!parser) {
        fprintf(out, "Parser *(NULL)\n");
    } else {
        fprintf(out, "Parser *(%p) %s {\n",
            (const void*)parser,
            VALIDATE_PARSER(parser) ? "VALID" : "INVALID"
        );
        printf("  previous = %s\n", tokenTypeName(parser->previous.type));
        printf("  next = %s\n", tokenTypeName(parser->next.type));
        printf("  did_read_next = %s\n", parser->did_read_next ? "true" : "false");
        printf("  lexer = ");
        fdumpLexer(out, parser->lexer, padding + 1);
        printf("  chunk = ");
        fdumpStack(out, parser->chunk, padding + 1);
        printf("  scope = ");
        fdumpScope(out, parser->scope, padding + 1);
        printf("}\n");
    }

#undef printf
}

void parse(Parser* parser) {
    ASSERT_PARSER(parser);

    while (
        peekNext(parser) != TOKEN_END &&
        peekNext(parser) != TOKEN_ERROR
    ) {
        parseDeclaration(parser);
    }

    ASSERT_PARSER(parser);
}

ValueType parseExpression(Parser* parser) {
    ASSERT_PARSER(parser);

    ValueType value_type = parseOr(parser);

    ASSERT_PARSER(parser);
    return value_type;
}


// ┌─────────────────────────────────┐
// │ Static function implementations │
// └─────────────────────────────────┘


// —————————————————————
//  Parser manipulation
// —————————————————————

static void readNext(Parser* parser) {
    ASSERT_PARSER(parser);
    assert(!parser->did_read_next);

    parser->next = readToken(parser->lexer);
    parser->did_read_next = true;

    ASSERT_PARSER(parser);
}

static TokenType advance(Parser* parser) {
    ASSERT_PARSER(parser);

    if (parser->did_read_next) {
        parser->previous = parser->next;
        parser->did_read_next = false;
    } else {
        parser->previous = readToken(parser->lexer);
    }

    ASSERT_PARSER(parser);
    return parser->previous.type;
}

static bool match(Parser* parser, TokenType expected) {
    ASSERT_PARSER(parser);

    bool matched = peekNext(parser) == expected;
    if (matched) {
        advance(parser);
    }

    ASSERT_PARSER(parser);
    return matched;
}

static Token forceMatch(Parser* parser, TokenType expected) {
    ASSERT_PARSER(parser);

    if (!match(parser, expected)) {
        // TODO: error
        printf("Expected %s, got %s\n", tokenTypeName(expected), tokenTypeName(peekNext(parser)));
        dumpParser(parser);
        assert(false);
    }

    return previous(parser);
}

static Token previous(const Parser* parser) {
    ASSERT_PARSER(parser);
    return parser->previous;
}

static TokenType peekNext(Parser* parser) {
    ASSERT_PARSER(parser);

    if (!parser->did_read_next) {
        readNext(parser);
    }
    return parser->next.type;
}


// —————————
//  Parsing
// —————————

static void parseDeclaration(Parser* parser) {
    ASSERT_PARSER(parser);

    switch (peekNext(parser)) {
        case TOKEN_VAR:   parseVariable(parser);  break;
        default:          parseStatement(parser); break;
    }

    ASSERT_PARSER(parser);
}

static void parseVariable(Parser* parser) {
    ASSERT_PARSER(parser);

    forceMatch(parser, TOKEN_VAR);
    Token identifier_token = forceMatch(parser, TOKEN_IDENTIFIER);

    // Type
    ValueType value_type;
    forceMatch(parser, TOKEN_COLON);
    switch (advance(parser)) {
        case TOKEN_BOOL:  value_type = VALUE_BOOL;  break;
        case TOKEN_INT:   value_type = VALUE_INT;   break;
        case TOKEN_FLOAT: value_type = VALUE_FLOAT; break;
        default:
            // not supported yet
            dumpParser(parser);
            assert(false);
    }

    // Initialization
    if (match(parser, TOKEN_EQUAL)) {
        ValueType initializer_value_type = parseExpression(parser);
        if (value_type != initializer_value_type) {
            // TODO: error
            printf("Initializer expression's type doesn't match the variable's type.\n");
            assert(false);
        }
    }
    // Default value
    else {
        switch (value_type) {
            case VALUE_BOOL:
                pushOpCodeOnStack(parser->chunk, OP_PUSH_FALSE);
                break;
            case VALUE_INT:
                pushOpCodeOnStack(parser->chunk, OP_PUSH_INT);
                pushIntOnStack(parser->chunk, (int32_t)0);
                break;
            case VALUE_FLOAT:
                pushOpCodeOnStack(parser->chunk, OP_PUSH_FLOAT);
                pushFloatOnStack(parser->chunk, (double)0);
                break;
            default: assert(false); // not supported yet
        }
    }

    if (!declareVariableInScope(
        parser->scope,
        identifier_token.start,
        identifier_token.length,
        value_type
    )) {
        // TODO: error
        assert(false);
    }

    ASSERT_PARSER(parser);
}

static void parseStatement(Parser* parser) {
    ASSERT_PARSER(parser);

    switch (peekNext(parser)) {
        case TOKEN_PRINT: parsePrint(parser);     break;
        case TOKEN_IDENTIFIER: parseAssignment(parser); break;
        default: assert(false); // Not implemented yet or error
    }

    ASSERT_PARSER(parser);
}

static void parsePrint(Parser* parser) {
    ASSERT_PARSER(parser);

    forceMatch(parser, TOKEN_PRINT);
    ValueType value_type = parseExpression(parser);
    switch (value_type) {
        case VALUE_BOOL:  pushOpCodeOnStack(parser->chunk, OP_PRINT_BOOL);  break;
        case VALUE_INT:   pushOpCodeOnStack(parser->chunk, OP_PRINT_INT);   break;
        case VALUE_FLOAT: pushOpCodeOnStack(parser->chunk, OP_PRINT_FLOAT); break;
        default:
            assert(false);
    }

    ASSERT_PARSER(parser);
}

static void parseAssignment(Parser* parser) {
    ASSERT_PARSER(parser);

    Token identifier_token = forceMatch(parser, TOKEN_IDENTIFIER);
    forceMatch(parser, TOKEN_EQUAL);
    ValueType expression_value_type = parseExpression(parser);

    Variable variable;
    bool found_variable = accessVariableInScope(
        parser->scope,
        identifier_token.start,
        identifier_token.length,
        &variable
    );

    if (!found_variable) {
        // TODO: error
        assert(false);
    }

    if (variable.type != expression_value_type) {
        // TODO: error;
        assert(false);
    }

    OpCode op_code;
    switch (variable.type) {
        case VALUE_BOOL:  op_code = OP_SET_BYTE_ON_STACK;  break;
        case VALUE_INT:   op_code = OP_SET_INT_ON_STACK;   break;
        case VALUE_FLOAT: op_code = OP_SET_FLOAT_ON_STACK; break;
        default: assert(false); // TODO: error
    }

    pushOpCodeOnStack(parser->chunk, op_code);
    pushAddressOnStack(parser->chunk, variable.address_on_stack);

    ASSERT_PARSER(parser);
}

static ValueType parseOr(Parser* parser) {
    ASSERT_PARSER(parser);

    ValueType value_type_l = parseAnd(parser);

    while (match(parser, TOKEN_OR)) {
        ValueType value_type_r = parseAnd(parser);

        validateOperatorTypes(TOKEN_OR, value_type_l, value_type_r);
        emitOpCodesForTokenAndValueTypesCombination(parser, 2, TOKEN_OR, value_type_l);
    }

    ASSERT_PARSER(parser);
    return value_type_l;
}

static ValueType parseAnd(Parser* parser) {
    ASSERT_PARSER(parser);

    ValueType value_type_l = parseComparison(parser);
    
    while (match(parser, TOKEN_AND)) {
        ValueType value_type_r = parseComparison(parser);

        validateOperatorTypes(TOKEN_AND, value_type_l, value_type_r);
        emitOpCodesForTokenAndValueTypesCombination(parser, 2, TOKEN_AND, value_type_l);
    }

    ASSERT_PARSER(parser);
    return value_type_l;
}

static ValueType parseComparison(Parser* parser) {
    ASSERT_PARSER(parser);

    ValueType value_type_l = parseTerm(parser);
    
    if (match(parser, TOKEN_EQUAL_EQUAL)       ||
        match(parser, TOKEN_EXCLAMATION_EQUAL) ||
        match(parser, TOKEN_GREATER_EQUAL)     ||
        match(parser, TOKEN_LESS_EQUAL)        ||
        match(parser, TOKEN_GREATER)           ||
        match(parser, TOKEN_LESS)
    ) {
        TokenType operator_token_type = previous(parser).type;
        ValueType value_type_r = parseTerm(parser);

        validateOperatorTypes(operator_token_type, value_type_l, value_type_r);
        emitOpCodesForTokenAndValueTypesCombination(parser, 2, operator_token_type, value_type_l);

        value_type_l = VALUE_BOOL;
    }

    ASSERT_PARSER(parser);
    return value_type_l;
}

static ValueType parseTerm(Parser* parser) {
    ASSERT_PARSER(parser);

    ValueType value_type_l = parseFactor(parser);
    
    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS)) {
        TokenType operator_token_type = previous(parser).type;
        ValueType value_type_r = parseFactor(parser);

        validateOperatorTypes(operator_token_type, value_type_l, value_type_r);
        emitOpCodesForTokenAndValueTypesCombination(parser, 2, operator_token_type, value_type_l);
    }

    ASSERT_PARSER(parser);
    return value_type_l;
}

static ValueType parseFactor(Parser* parser) {
    ASSERT_PARSER(parser);

    ValueType value_type_l = parsePrefix(parser);
    
    while (
        match(parser, TOKEN_STAR)    ||
        match(parser, TOKEN_SLASH)   || 
        match(parser, TOKEN_PERCENT)
    ) {
        TokenType operator_token_type = previous(parser).type;
        ValueType value_type_r = parsePrefix(parser);

        validateOperatorTypes(operator_token_type, value_type_l, value_type_r);
        emitOpCodesForTokenAndValueTypesCombination(parser, 2, operator_token_type, value_type_l);
    }

    ASSERT_PARSER(parser);
    return value_type_l;
}

static ValueType parsePrefix(Parser* parser) {
    ASSERT_PARSER(parser);

    bool had_prefix_operator = false;
    TokenType operator_token_type;
    if (match(parser, TOKEN_MINUS) || match(parser, TOKEN_EXCLAMATION)) {
        had_prefix_operator = true;
        operator_token_type = previous(parser).type;
    }
    
    ValueType value_type = parsePostfix(parser);

    if (had_prefix_operator) {
        validateOperatorTypes(operator_token_type, value_type, value_type);
        emitOpCodesForTokenAndValueTypesCombination(parser, 1, operator_token_type, value_type);
    }

    ASSERT_PARSER(parser);
    return value_type;
}

static ValueType parsePostfix(Parser* parser) {
    ASSERT_PARSER(parser);

    ValueType value_type = parsePrimary(parser);

    while (
        match(parser, TOKEN_DOT)      ||
        match(parser, TOKEN_LPAREN)   ||
        match(parser, TOKEN_LBRACKET) ||
        match(parser, TOKEN_COLON)
    ) {
        switch (previous(parser).type) {
            
            // Member access
            case TOKEN_DOT:
                // TODO: structs not implemented yet
                assert(false);
                // TODO: 1. check type is obj
                //       2. eat identifier
                //       3. push member access op

            // Call
            case TOKEN_LPAREN:
                // TODO: functions not implemented yet
                assert(false);
                // TODO: 1. check type is func
                //       2. eat argument list
                //       3. check signature
                //       4. push call op
            
            // Subscript
            case TOKEN_LBRACKET:
                // TODO: arrays and maps are not implemented yet
                assert(false);
                // TODO: 1. check type is arr or map
                //       2. eat index
                //       3. push subscript op

            // Type cast
            case TOKEN_COLON:
                switch (advance(parser)) {
                    
                    case TOKEN_INT: {
                        ValueType expected[4] = { VALUE_FLOAT };
                        validateOperandType(value_type, expected);
                        pushOpCodeOnStack(parser->chunk, OP_CAST_FLOAT_TO_INT);
                        value_type = VALUE_INT;
                        break;
                    }
                    
                    case TOKEN_FLOAT: {
                        ValueType expected[4] = { VALUE_INT };
                        validateOperandType(value_type, expected);
                        pushOpCodeOnStack(parser->chunk, OP_CAST_INT_TO_FLOAT);
                        value_type = VALUE_FLOAT;
                        break;
                    }
                    
                    case TOKEN_STRING:
                        // TODO: not implemented yet
                        assert(false);

                    default:
                        // TODO: error
                        assert(false);
                }
                break;

            default:
                assert(false);
        }
    }

    ASSERT_PARSER(parser);
    return value_type;
}

static ValueType parsePrimary(Parser* parser) {
    ASSERT_PARSER(parser);

    ValueType value_type;

    switch (advance(parser)) {

        case TOKEN_TRUE:
            pushOpCodeOnStack(parser->chunk, OP_PUSH_TRUE);
            value_type = VALUE_BOOL;
            break;
        
        case TOKEN_FALSE:
            pushOpCodeOnStack(parser->chunk, OP_PUSH_FALSE);
            value_type = VALUE_BOOL;
            break;
        
        case TOKEN_INTEGER_VALUE: {
            int32_t value = (int32_t)strtol(previous(parser).start, NULL, 10);
            pushOpCodeOnStack(parser->chunk, OP_PUSH_INT);
            pushIntOnStack(parser->chunk, value);
            value_type = VALUE_INT;
            break;
        }
        
        case TOKEN_FLOAT_VALUE: {
            double value = strtod(previous(parser).start, NULL);
            pushOpCodeOnStack(parser->chunk, OP_PUSH_FLOAT);
            pushFloatOnStack(parser->chunk, value);
            value_type = VALUE_FLOAT;
            break;
        }
        
        case TOKEN_STRING_VALUE:
            pushOpCodeOnStack(parser->chunk, OP_ACCESS_CONSTANT_TABLE);
            uint8_t constant_table_address = addStringConstant(previous(parser).start, previous(parser).length);
            pushByteOnStack(parser->chunk, constant_table_address);
            value_type = VALUE_STRING;
            break;
        
        case TOKEN_IDENTIFIER: {
            Variable variable;
            bool found_variable = accessVariableInScope(
                parser->scope,
                previous(parser).start,
                previous(parser).length,
                &variable
            );

            if (!found_variable) {
                // TODO: error
                assert(false);
            }

            OpCode op_code;
            switch (variable.type) {
                case VALUE_BOOL:  op_code = OP_GET_BYTE_FROM_STACK;  break;
                case VALUE_INT:   op_code = OP_GET_INT_FROM_STACK;   break;
                case VALUE_FLOAT: op_code = OP_GET_FLOAT_FROM_STACK; break;
                default: assert(false); // TODO: error
            }

            pushOpCodeOnStack(parser->chunk, op_code);
            pushAddressOnStack(parser->chunk, variable.address_on_stack);

            value_type = variable.type;
            break;
        }
        
        case TOKEN_LBRACKET:
            // TODO: array; not implemented yet
            assert(false);
        
        case TOKEN_LBRACE:
            // TODO: map; not implemented yet
            assert(false);
        
        case TOKEN_LPAREN:
            value_type = parseExpression(parser);
            forceMatch(parser, TOKEN_RPAREN);
            break;
        
        default:
            // TODO: error
            printf("%s\n", tokenTypeName(previous(parser).type));
            dumpParser(parser);
            assert(false);
    }

    ASSERT_PARSER(parser);
    return value_type;
}


// ─────────────────────
//  Operator type rules 
// ─────────────────────

typedef struct {
    ValueType  left_operand_expected_types[4];
    ValueType right_operand_expected_types[4];
    bool types_have_to_match;
} OperatorTypeRules;

#define ANY { VALUE_BOOL, VALUE_INT, VALUE_FLOAT, VALUE_STRING }
#define B   { VALUE_BOOL, VALUE_BOOL, VALUE_BOOL, VALUE_BOOL }
#define I   { VALUE_INT, VALUE_INT, VALUE_INT, VALUE_INT }
#define IF  { VALUE_INT, VALUE_FLOAT, VALUE_FLOAT, VALUE_FLOAT }
#define IFS { VALUE_INT, VALUE_FLOAT, VALUE_STRING, VALUE_STRING }

OperatorTypeRules operator_type_rules[] = {
    [TOKEN_OR]                = { B,    B,    true  },
    [TOKEN_AND]               = { B,    B,    true  },
    [TOKEN_EQUAL_EQUAL]       = { ANY,  ANY,  true  },
    [TOKEN_EXCLAMATION_EQUAL] = { ANY,  ANY,  true  },
    [TOKEN_GREATER_EQUAL]     = { IFS,  IFS,  true  },
    [TOKEN_LESS_EQUAL]        = { IFS,  IFS,  true  },
    [TOKEN_GREATER]           = { IFS,  IFS,  true  },
    [TOKEN_LESS]              = { IFS,  IFS,  true  },
    [TOKEN_PLUS]              = { IFS,  IFS,  true  },
    [TOKEN_MINUS]             = { IF,   IF,   true  },
    [TOKEN_STAR]              = { IF,   IF,   true  },
    [TOKEN_SLASH]             = { IF,   IF,   true  },
    [TOKEN_PERCENT]           = { IF,   I,    false },
    [TOKEN_EXCLAMATION]       = { B,    B,    true },
};

#undef IFS
#undef IF
#undef I
#undef B
#undef ANY

static bool validateOperandType(
    ValueType operand_type,
    ValueType expected_operand_types[4]
) {
    bool operand_type_matched = false;
    for (size_t i = 0; i < 4; ++i) {
        if (expected_operand_types[i] == operand_type) {
            operand_type_matched = true;
        }
    }

    if (!operand_type_matched) {
        // TODO: error
        return false;
    }
    
    return true;
}

static bool validateOperatorTypes(
    TokenType operator,
    ValueType left_operand_type,
    ValueType right_operand_type
) {
    OperatorTypeRules rules = operator_type_rules[operator];

    if (!validateOperandType(left_operand_type,  rules.left_operand_expected_types) ||
        !validateOperandType(right_operand_type, rules.right_operand_expected_types)
    ){
        return false;
    }

    if (rules.types_have_to_match && left_operand_type != right_operand_type) {
        // TODO: error
        return false;
    }

    return true;
}


// ────────────────────
//  Operator to OpCode 
// ────────────────────

#define KEY(arity, token_type, value_type) ((token_type * 4 + value_type) * 3 + arity)

OpCode token_and_value_type_to_opcodes[][2] = {
    [KEY(2, TOKEN_OR,                 VALUE_BOOL)  ] = { OP_OR,              OP_EMPTY       },

    [KEY(2, TOKEN_AND,                VALUE_BOOL)  ] = { OP_AND,             OP_EMPTY       },
    
    [KEY(2, TOKEN_EQUAL_EQUAL,        VALUE_BOOL)  ] = { OP_EQUALS_BOOL,     OP_EMPTY       },
    [KEY(2, TOKEN_EQUAL_EQUAL,        VALUE_INT)   ] = { OP_EQUALS_INT,      OP_EMPTY       },
    [KEY(2, TOKEN_EQUAL_EQUAL,        VALUE_FLOAT) ] = { OP_EQUALS_FLOAT,    OP_EMPTY       },
    [KEY(2, TOKEN_EQUAL_EQUAL,        VALUE_STRING)] = { OP_EQUALS_STRING,   OP_EMPTY       },
    
    [KEY(2, TOKEN_EXCLAMATION_EQUAL,  VALUE_BOOL)  ] = { OP_EQUALS_BOOL,     OP_NEGATE_BOOL },
    [KEY(2, TOKEN_EXCLAMATION_EQUAL,  VALUE_INT)   ] = { OP_EQUALS_INT,      OP_NEGATE_BOOL },
    [KEY(2, TOKEN_EXCLAMATION_EQUAL,  VALUE_FLOAT) ] = { OP_EQUALS_FLOAT,    OP_NEGATE_BOOL },
    [KEY(2, TOKEN_EXCLAMATION_EQUAL,  VALUE_STRING)] = { OP_EQUALS_STRING,   OP_NEGATE_BOOL },
    
    [KEY(2, TOKEN_GREATER_EQUAL,      VALUE_INT)   ] = { OP_LESS_INT,        OP_NEGATE_BOOL },
    [KEY(2, TOKEN_GREATER_EQUAL,      VALUE_FLOAT) ] = { OP_LESS_FLOAT,      OP_NEGATE_BOOL },
    [KEY(2, TOKEN_GREATER_EQUAL,      VALUE_STRING)] = { OP_LESS_STRING,     OP_NEGATE_BOOL },
    
    [KEY(2, TOKEN_LESS_EQUAL,         VALUE_INT)   ] = { OP_GREATER_INT,     OP_NEGATE_BOOL },
    [KEY(2, TOKEN_LESS_EQUAL,         VALUE_FLOAT) ] = { OP_GREATER_FLOAT,   OP_NEGATE_BOOL },
    [KEY(2, TOKEN_LESS_EQUAL,         VALUE_STRING)] = { OP_GREATER_STRING,  OP_NEGATE_BOOL },
    
    [KEY(2, TOKEN_GREATER,            VALUE_INT)   ] = { OP_GREATER_INT,     OP_EMPTY       },
    [KEY(2, TOKEN_GREATER,            VALUE_FLOAT) ] = { OP_GREATER_FLOAT,   OP_EMPTY       },
    [KEY(2, TOKEN_GREATER,            VALUE_STRING)] = { OP_GREATER_STRING,  OP_EMPTY       },
    
    [KEY(2, TOKEN_LESS,               VALUE_INT)   ] = { OP_LESS_INT,        OP_EMPTY       },
    [KEY(2, TOKEN_LESS,               VALUE_FLOAT) ] = { OP_LESS_FLOAT,      OP_EMPTY       },
    [KEY(2, TOKEN_LESS,               VALUE_STRING)] = { OP_LESS_STRING,     OP_EMPTY       },
    
    [KEY(2, TOKEN_PLUS,               VALUE_INT)   ] = { OP_ADD_INT,         OP_EMPTY       },
    [KEY(2, TOKEN_PLUS,               VALUE_FLOAT) ] = { OP_ADD_FLOAT,       OP_EMPTY       },
    [KEY(2, TOKEN_PLUS,               VALUE_STRING)] = { OP_CONCATENATE,     OP_EMPTY       },
    
    [KEY(2, TOKEN_MINUS,              VALUE_INT)   ] = { OP_NEGATE_INT,      OP_ADD_INT     },
    [KEY(2, TOKEN_MINUS,              VALUE_FLOAT) ] = { OP_NEGATE_FLOAT,    OP_ADD_FLOAT   },
    
    [KEY(2, TOKEN_STAR,               VALUE_INT)   ] = { OP_MULTIPLY_INT,    OP_EMPTY       },
    [KEY(2, TOKEN_STAR,               VALUE_FLOAT) ] = { OP_MULTIPLY_FLOAT,  OP_EMPTY       },
    
    [KEY(2, TOKEN_SLASH,              VALUE_INT)   ] = { OP_DIVIDE_INT,      OP_EMPTY       },
    [KEY(2, TOKEN_SLASH,              VALUE_FLOAT) ] = { OP_DIVIDE_FLOAT,    OP_EMPTY       },
    
    [KEY(2, TOKEN_PERCENT,            VALUE_INT)   ] = { OP_MODULO_INT,      OP_EMPTY       },
    [KEY(2, TOKEN_PERCENT,            VALUE_FLOAT) ] = { OP_MODULO_FLOAT,    OP_EMPTY       },

    [KEY(1, TOKEN_EXCLAMATION,        VALUE_BOOL)  ] = { OP_NEGATE_BOOL,     OP_EMPTY       },
    [KEY(1, TOKEN_MINUS,              VALUE_INT)   ] = { OP_NEGATE_INT,      OP_EMPTY       },
    [KEY(1, TOKEN_MINUS,              VALUE_FLOAT) ] = { OP_NEGATE_FLOAT,    OP_EMPTY       },
};

static void emitOpCodesForTokenAndValueTypesCombination(
    Parser* parser,
    uint8_t arity,
    TokenType token_type,
    ValueType value_type
) {
    pushOpCodeOnStack(
        parser->chunk,
        token_and_value_type_to_opcodes[KEY(arity, token_type, value_type)][0]
    );
    pushOpCodeOnStack(
        parser->chunk,
        token_and_value_type_to_opcodes[KEY(arity, token_type, value_type)][1]
    );
}

#undef KEY


#undef VALIDATE_PARSER
#undef ASSERT_PARSER

