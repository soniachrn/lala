#include "parser.h"


#include <assert.h>
#include <stdlib.h>

#include "ccf.h"
#include "debug.h"
#include "heap.h"


// ┌────────┐
// │ Macros │
// └────────┘

#define VALIDATE_HALF_INITIALIZED_PARSER(parser) \
    (                                            \
        parser &&                                \
        parser->chunk                            \
    )

#define VALIDATE_PARSER(parser) \
    (                           \
        parser &&               \
        parser->lexer &&        \
        parser->chunk           \
    )

#define ASSERT_HALF_INITIALIZED_PARSER(parser)           \
    if (!VALIDATE_HALF_INITIALIZED_PARSER(parser)) {     \
        fprintf(stderr,                                  \
            "%s:%d, in %s:\nParser assertion failed.\n", \
            __FILENAME__,                                \
            __LINE__,                                    \
            __FUNCTION_NAME__                            \
        );                                               \
        fdumpParser(stderr, parser, 0);                  \
        exit(1);                                         \
    }

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


// ┌──────────────────────────────┐
// │ Static function declarations │
// └──────────────────────────────┘


// —————————————————————
//  Parser manipulation
// —————————————————————

// Reads the next token, but doesn't consume it.
static void readNext(Parser* parser);

// Return the last consumed token.
static Token previous(const Parser* parser);

static Token next(Parser* parser);

// Returns the type of the next token without consuming it.
static TokenType peekNext(Parser* parser);

// Advances and returns the consumed token's type.
static TokenType advance(Parser* parser);

// If the current token's type is token_type, advances and returns
// true. Else, returns false.
static bool match(Parser* parser, TokenType expected);

// If the current token's type is token_type, advances and returns 
// the matched token. Else, results in an error.
static Token forceMatch(Parser* parser, TokenType expected);

// Skips tokens until encounters a declration or a statement start.
static void synchronize(Parser* parser);


// —————————
//  Parsing
// —————————

// Declaration
static StatementProperties parseGlobalStatement(Parser* parser);
static StatementProperties parseInclude(Parser* parser);
static StatementProperties parseDeclaration(Parser* parser);
static StatementProperties parseVariable(Parser* parser);
static StatementProperties parseFunction(Parser* parser);
static StatementProperties parseStructure(Parser* parser);
static ValueType* parseValueType(Parser* parser);

// Statement
static StatementProperties parseStatement (Parser* parser);
static StatementProperties parsePrint     (Parser* parser);
static StatementProperties parseIf        (Parser* parser);
static StatementProperties parseWhile     (Parser* parser);
static StatementProperties parseDoWhile   (Parser* parser);
static StatementProperties parseContinue  (Parser* parser);
static StatementProperties parseBreak     (Parser* parser);
static StatementProperties parseReturn    (Parser* parser);
static StatementProperties parseBlock     (Parser* parser);

// Expression
static ValueType* parseOr        (Parser* parser);
static ValueType* parseAnd       (Parser* parser);
static ValueType* parseComparison(Parser* parser);
static ValueType* parseTerm      (Parser* parser);
static ValueType* parseFactor    (Parser* parser);
static ValueType* parsePrefix    (Parser* parser);
static ValueType* parsePostfix   (Parser* parser, ExpressionKind expression_kind);
static ValueType* parsePrimary   (Parser* parser, ExpressionKind expression_kind);


// —————————————————————
//  Operator type rules 
// —————————————————————

static bool validateOperandType(
    BasicValueType operand_type,
    BasicValueType expected_operand_types[4]
);

static void validateOperatorTypes(
    Parser* parser,
    Token expression_start_token,
    TokenType operator,
    BasicValueType left_operand_type,
    BasicValueType right_operand_type
);


// ────────────────────
//  Operator to OpCode 
// ────────────────────

static void emitOpCodesForTokenAndValueTypesCombination(
    Parser* parser,
    uint8_t arity,
    TokenType token_type,
    BasicValueType basic_value_type
);

static void pushOpCodeOnStack(Stack* stack, OpCode op_code) {
    if (op_code != OP_EMPTY) {
        pushByteOnStack(stack, (uint8_t)op_code);
    }
}

OpCode getOpGetFromStackForValueType(
    ValueType* value_type, 
    VariableKind kind
);
OpCode getOpSetOnStackForValueType(
    ValueType* value_type,
    VariableKind kind
);


// ┌──────────────────────────┐
// │ Function implementations │
// └──────────────────────────┘

void initParser(Parser* parser, Stack* chunk) {
    assert(parser);
    assert(chunk);

    initHashMap(&parser->includes);
    parser->lexer = NULL;
    parser->chunk = chunk;
    parser->did_read_next = false;
    parser->panic_mode = false;
    parser->had_error = false;
    parser->scope = createScope(NULL);
    parser->constants.count = 0;
    initStack(&parser->free_on_end);

    ASSERT_HALF_INITIALIZED_PARSER(parser);
}

void freeParser(Parser* parser) {
    ASSERT_HALF_INITIALIZED_PARSER(parser);

    freeHashMap(&parser->includes);
    parser->lexer = NULL;
    parser->chunk = NULL;
    parser->did_read_next = false;
    deleteScope(parser->scope);

    while (stackSize(&parser->free_on_end) > 0) {
        free((void*)popAddressFromStack(&parser->free_on_end));
    }
    freeStack(&parser->free_on_end);
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
            VALIDATE_HALF_INITIALIZED_PARSER(parser) ? "VALID" : "INVALID"
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

ParseFileResult parseFile(Parser* parser, const char* file_path) {
    ASSERT_HALF_INITIALIZED_PARSER(parser);
    
    // Get the include state of the file.
    size_t file_path_length = strlen(file_path);
    // Avoid 'converting a packed pointer to a size_t pointer may
    // result in an unaligned pointer value' warning
    size_t include_state_size_t = (size_t)INCLUDE_NOT_STARTED;
    getFromHashMap(
        &parser->includes,
        file_path,
        file_path_length,
        &include_state_size_t
    );
    IncludeState include_state = (IncludeState)include_state_size_t;

    // If inclusion is in process already, a recursive inclusion
    // occurred, which results in a compile error.
    if (include_state == INCLUDE_IN_PROCESS) {
        return (ParseFileResult){
            PARSE_FILE_RECURSIVE_INCLUDE,
            READ_FILE_SUCCESS
        };
    }

    // If this file was not included before, parse it.
    else if (include_state == INCLUDE_NOT_STARTED) {
        storeInHashMap(
            &parser->includes,
            file_path,
            file_path_length,
            INCLUDE_IN_PROCESS
        );

        char* source = NULL;
        size_t source_length = 0;
        ReadFileResult read_file_result = readFile(file_path, &source, &source_length);
        if (read_file_result != READ_FILE_SUCCESS) {
            return (ParseFileResult){
                PARSE_FILE_READ_FILE_ERROR,
                read_file_result
            };
        }
        pushAddressOnStack(&parser->free_on_end, (size_t)source);

        parseString(parser, source);

        storeInHashMap(
            &parser->includes,
            file_path,
            file_path_length,
            INCLUDE_FINISHED
        );
    }

    // If this file was included before already, don't do anything.
    else { }

    ASSERT_HALF_INITIALIZED_PARSER(parser);
    return (ParseFileResult){
        PARSE_FILE_SUCCESS,
        READ_FILE_SUCCESS
    };
}

void parseString(Parser* parser, const char* source) {
    ASSERT_HALF_INITIALIZED_PARSER(parser);

    Lexer new_lexer;
    initLexer(&new_lexer, source);

    Lexer* old_lexer          = parser->lexer;
    Token  old_previous_token = parser->previous;
    Token  old_next_token     = parser->next;
    bool   old_did_read_next  = parser->did_read_next;

    parser->lexer         = &new_lexer;
    parser->did_read_next = false;

    parse(parser);

    parser->lexer         = old_lexer;
    parser->previous      = old_previous_token;
    parser->next          = old_next_token;
    parser->did_read_next = old_did_read_next;

    freeLexer(&new_lexer);

    ASSERT_HALF_INITIALIZED_PARSER(parser);
}

void parse(Parser* parser) {
    ASSERT_PARSER(parser);

    while (peekNext(parser) != TOKEN_END) {
        parseGlobalStatement(parser);
    }

    ASSERT_PARSER(parser);
}

ValueType* parseExpression(Parser* parser) {
    ASSERT_PARSER(parser);
    return parseOr(parser);
}


// ┌─────────────────────────────────┐
// │ Static function implementations │
// └─────────────────────────────────┘


// —————————————————————
//  Parser manipulation
// —————————————————————

#define error(parser, error_kind, start_token, end_token, ...)  \
    {                                                           \
        if (!parser->panic_mode) {                              \
            parser->panic_mode = true;                          \
            parser->had_error  = true;                          \
                                                                \
            /* Print error title and position. */               \
            fprintf(                                            \
                stderr,                                         \
                CCF_RED("%s error at %d:%d:\n"),                \
                error_kind,                                     \
                start_token.line,                               \
                start_token.symbol                              \
            );                                                  \
                                                                \
            /* Print error message */                           \
            fprintf(stderr, __VA_ARGS__);                       \
            fprintf(stderr, "\n");                              \
                                                                \
            /* Print source code line */                        \
            uint8_t line_length = fprintLine(                   \
                stderr,                                         \
                parser->lexer,                                  \
                start_token.line                                \
            );                                                  \
                                                                \
            /* Highlight the error tokens */                    \
            fprintf(stderr, "%*s", start_token.symbol - 1, ""); \
            uint8_t highlight_length;                           \
            if (start_token.line == end_token.line) {           \
                highlight_length = (                            \
                    (uint8_t)(end_token.start -                 \
                    start_token.start) +                        \
                    end_token.length                            \
                );                                              \
                if (end_token.type == TOKEN_ERROR) {            \
                    highlight_length = (uint8_t)(               \
                        parser->lexer->current -                \
                        parser->lexer->token_start              \
                    );                                          \
                }                                               \
            } else {                                            \
                highlight_length = (uint8_t)(                   \
                    line_length -                               \
                    (start_token.symbol - 1)                    \
                );                                              \
            }                                                   \
            for (uint8_t i = 0; i < highlight_length; ++i) {    \
                fputc('~', stderr);                             \
            }                                                   \
            fputc('\n', stderr);                                \
        }                                                       \
    }

#define errorAt(parser, error_kind, token, ...)  error(  parser, error_kind, token, token,     __VA_ARGS__)
#define errorAtPrevious(parser, error_kind, ...) errorAt(parser, error_kind, previous(parser), __VA_ARGS__)
#define errorAtNext(    parser, error_kind, ...) errorAt(parser, error_kind, next(parser),     __VA_ARGS__)

static void readNext(Parser* parser) {
    ASSERT_PARSER(parser);
    assert(!parser->did_read_next);

    parser->did_read_next = true;

    parser->next = readToken(parser->lexer);
    // while (parser->next.type == TOKEN_ERROR) {
    if (parser->next.type == TOKEN_ERROR) {
        errorAtNext(parser, "Lexical", "%.*s", parser->next.length, parser->next.start);
        // parser->next = readToken(parser->lexer);
    }

    ASSERT_PARSER(parser);
}

static Token previous(const Parser* parser) {
    ASSERT_PARSER(parser);
    return parser->previous;
}

static Token next(Parser* parser) {
    ASSERT_PARSER(parser);

    if (!parser->did_read_next) {
        readNext(parser);
    }
    return parser->next;
}

static TokenType peekNext(Parser* parser) {
    ASSERT_PARSER(parser);

    if (!parser->did_read_next) {
        readNext(parser);
    }
    return parser->next.type;
}

static TokenType advance(Parser* parser) {
    ASSERT_PARSER(parser);

    if (parser->did_read_next) {
        parser->previous = parser->next;
        parser->did_read_next = false;
    } else {
        parser->previous = readToken(parser->lexer);
        if (parser->previous.type == TOKEN_ERROR) {
            errorAtPrevious(parser, "Lexical", "%.*s", parser->previous.length, parser->previous.start);
        }
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
        errorAtNext(
            parser,
            "Syntactic",
            "Expected %s, got %s.",
            tokenTypeName(expected),
            tokenTypeName(peekNext(parser))
        );
    }

    return previous(parser);
}

static void synchronize(Parser* parser) {
    ASSERT_PARSER(parser);
    assert(parser->panic_mode);

    parser->panic_mode = false;

    while (true) {
        switch (peekNext(parser)) {
            case TOKEN_INCLUDE:
            case TOKEN_VAR:
            case TOKEN_FUNCTION:
            case TOKEN_STRUCTURE:
            case TOKEN_PRINT:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_CONTINUE:
            case TOKEN_BREAK:
            case TOKEN_RETURN:
            case TOKEN_END:
                ASSERT_PARSER(parser);
                return;
            default:
                advance(parser);
                break;
        }
    }
}


// —————————
//  Parsing
// —————————

static StatementProperties parseGlobalStatement(Parser* parser) {
    ASSERT_PARSER(parser);
    
    StatementProperties statement_properties = { false };

    switch (peekNext(parser)) {
        case TOKEN_INCLUDE: statement_properties = parseInclude(parser);     break;
        default:            statement_properties = parseDeclaration(parser); break;
    }

    if (parser->panic_mode) {
        synchronize(parser);
    }

    ASSERT_PARSER(parser);
    return statement_properties;
}

static StatementProperties parseInclude(Parser* parser) {
    ASSERT_PARSER(parser);

    Token statement_start_token = forceMatch(parser, TOKEN_INCLUDE);

    // Include file path.
    Token identifier = forceMatch(parser, TOKEN_IDENTIFIER);
    char* path = malloc(identifier.length + 1);
    memcpy(path, identifier.start, identifier.length);
    path[identifier.length] = '\0';

    while (match(parser, TOKEN_DOT)) {
        identifier = forceMatch(parser, TOKEN_IDENTIFIER);
        path = concatenatePath(path, identifier.start, identifier.length);
    }
    path = addExtensionToPath(path, "lala");
    pushAddressOnStack(&parser->free_on_end, (size_t)path);

    // Make sure parsed include file path without errors.
    if (parser->panic_mode) {
        ASSERT_PARSER(parser);
        StatementProperties statement_properties = { false };
        return statement_properties;
    }
    
    // Parse the include file.
    ParseFileResult parse_file_result = parseFile(parser, path);
    switch (parse_file_result.type) {
        case PARSE_FILE_RECURSIVE_INCLUDE:
            error(
                parser,
                "Semantic",
                statement_start_token,
                previous(parser),
                "Attempt to recursively include file %s.",
                path
            );
            break;
        case PARSE_FILE_READ_FILE_ERROR:
            error(
                parser,
                "File read",
                statement_start_token,
                previous(parser),
                "%s %s.",
                getReadFileResultErrorMessage(parse_file_result.read_file_result),
                path
            );
            break;
        case PARSE_FILE_SUCCESS:
            break;
        default:
            assert(false);
    }

    ASSERT_PARSER(parser);
    StatementProperties statement_properties = { false };
    return statement_properties;
}

static StatementProperties parseDeclaration(Parser* parser) {
    ASSERT_PARSER(parser);

    StatementProperties statement_properties = { false };

    switch (peekNext(parser)) {
        case TOKEN_VAR:       statement_properties = parseVariable(parser);  break;
        case TOKEN_FUNCTION:  statement_properties = parseFunction(parser);  break;
        case TOKEN_STRUCTURE: statement_properties = parseStructure(parser); break;
        default:              statement_properties = parseStatement(parser); break;
    }

    if (parser->panic_mode) {
        synchronize(parser);
    }

    ASSERT_PARSER(parser);
    return statement_properties;
}

static StatementProperties parseVariable(Parser* parser) {
    ASSERT_PARSER(parser);

    forceMatch(parser, TOKEN_VAR);
    Token identifier_token = forceMatch(parser, TOKEN_IDENTIFIER);

    // Type
    forceMatch(parser, TOKEN_COLON);
    ValueType* variable_type = parseValueType(parser);

    // Initialization
    forceMatch(parser, TOKEN_EQUAL);
    Token expression_start_token = next(parser);
    ValueType* initializer_value_type = parseExpression(parser);

    if (parser->panic_mode) {
        ASSERT_PARSER(parser);
        StatementProperties statement_properties = { false };
        return statement_properties;
    }

    if (!valueTypesEqual(variable_type, initializer_value_type)) {
        error(
            parser,
            "Semantic",
            expression_start_token,
            previous(parser),
            "Variable type (%s) and initializer expression type (%s) don't match.",
            valueTypeName(variable_type),
            valueTypeName(initializer_value_type)
        );
    }

    if (parser->panic_mode) {
        ASSERT_PARSER(parser);
        StatementProperties statement_properties = { false };
        return statement_properties;
    }

    // Declare variable
    switch (declareVariableInScope(
        parser->scope,
        identifier_token.start,
        identifier_token.length,
        variable_type
    )) {
        case VARDECL_SUCCESS:
            break;

        case VARDECL_TOO_MANY_VARIABLES_IN_A_SCOPE:
            errorAt(
                parser,
                "Semantic",
                identifier_token,
                "Could not declare variable %.*s. "
                "Can't declare more than %d variables in a scope.",
                identifier_token.length,
                identifier_token.start,
                MAX_VARIABLES_IN_SCOPE
            );
            break;

        case VARDECL_VARIABLE_REDECLARATION:
            errorAt(
                parser,
                "Semantic",
                identifier_token,
                "Variable %.*s redeclaration.",
                identifier_token.length,
                identifier_token.start
            );
            break;

        default:
            assert(false);
    }

    ASSERT_PARSER(parser);
    StatementProperties statement_properties = { false };
    return statement_properties;
}

static StatementProperties parseFunction(Parser* parser) {
    ASSERT_PARSER(parser);

    // Function name
    Token function_token = forceMatch(parser, TOKEN_FUNCTION);
    Token identifier_token = forceMatch(parser, TOKEN_IDENTIFIER);

    // Function type
    Scope* function_scope = createScopeInNewCallFrame(parser->scope);
    ValueType* function_type = createFunctionValueType();

    // Parameters
    forceMatch(parser, TOKEN_LPAREN);
    while (!match(parser, TOKEN_RPAREN)) {

        // Parameter name
        forceMatch(parser, TOKEN_VAR);
        Token parameter_identifier_token = forceMatch(parser, TOKEN_IDENTIFIER);

        // Parameter type
        forceMatch(parser, TOKEN_COLON);
        ValueType* parameter_type = parseValueType(parser);

        if (peekNext(parser) != TOKEN_RPAREN) {
            forceMatch(parser, TOKEN_COMMA);
        }

        // Add parameter to function type and declare parameter in scope
        addParameterToFunctionValueType(&function_type->as.function, parameter_type);
        switch (declareVariableInScope(
            function_scope,
            parameter_identifier_token.start,
            parameter_identifier_token.length,
            parameter_type
        )) {
            case VARDECL_SUCCESS:
                break;

            case VARDECL_TOO_MANY_VARIABLES_IN_A_SCOPE:
                errorAt(
                    parser,
                    "Semantic",
                    identifier_token,
                    "Could not declare parameter %.*s. "
                    "A function can't have more than %d parameters.",
                    identifier_token.length,
                    identifier_token.start,
                    MAX_VARIABLES_IN_SCOPE
                );
                break;

            case VARDECL_VARIABLE_REDECLARATION:
                errorAt(
                    parser,
                    "Semantic",
                    identifier_token,
                    "Parameter %.*s redeclares another variable.",
                    identifier_token.length,
                    identifier_token.start
                );
                break;

            default:
                assert(false);
        }
    }
    
    // Return type
    forceMatch(parser, TOKEN_COLON);
    ValueType* return_type = parseValueType(parser);

    if (parser->panic_mode) {
        ASSERT_PARSER(parser);
        StatementProperties statement_properties = { false };
        return statement_properties;
    }

    function_type->as.function.return_type = return_type;
    function_scope->return_type = return_type;

    // Declare function; fill the function start address value later.
    pushOpCodeOnStack(parser->chunk, OP_PUSH_ADDRESS);

    size_t function_start_address_position_in_chunk = stackSize(parser->chunk);
    pushAddressOnStack(parser->chunk, (size_t)0);

    pushOpCodeOnStack(parser->chunk, OP_DEFINE_ON_HEAP);
    pushAddressOnStack(parser->chunk, sizeof(size_t));
    pushByteOnStack(parser->chunk, REFERENCE_RULE_PLAIN);

    switch (declareVariableInScope(
        parser->scope,
        identifier_token.start,
        identifier_token.length,
        function_type
    )) {
        case VARDECL_SUCCESS:
            break;

        case VARDECL_TOO_MANY_VARIABLES_IN_A_SCOPE:
            errorAt(
                parser,
                "Semantic",
                identifier_token,
                "Could not declare function %.*s. "
                "Can't declare more than %d variables in a scope.",
                identifier_token.length,
                identifier_token.start,
                MAX_VARIABLES_IN_SCOPE
            );
            break;

        case VARDECL_VARIABLE_REDECLARATION:
            errorAt(
                parser,
                "Semantic",
                identifier_token,
                "Function %.*s redeclares another variable.",
                identifier_token.length,
                identifier_token.start
            );
            break;

        default:
            assert(false);
    }

    // Jump to after the function body; fill the address later
    pushOpCodeOnStack(parser->chunk, OP_JUMP);
    size_t after_body_address_position_in_chunk = stackSize(parser->chunk);
    pushAddressOnStack(parser->chunk, (size_t)0);

    // Fill the function start address
    size_t function_start_address = stackSize(parser->chunk);
    setAddressOnStack(parser->chunk, function_start_address_position_in_chunk, function_start_address);

    // Body
    parser->scope = function_scope;
    StatementProperties body_properties = parseStatement(parser);
    parser->scope = deleteScope(parser->scope);

    // Make sure the function is return-terminated.
    if (!body_properties.ends_with_return) {
        if (valueTypesEqual(return_type, &VALUE_TYPE_VOID)) {
            pushOpCodeOnStack(parser->chunk, OP_RETURN_VOID);
        } else {
            errorAt(
                parser,
                "Semantic",
                function_token,
                "Non-void function doesn't always end with a return."
            );
        }
    }

    // Fill the jump-after-the-body address
    size_t after_body_address = stackSize(parser->chunk);
    setAddressOnStack(parser->chunk, after_body_address_position_in_chunk, after_body_address);

    ASSERT_PARSER(parser);
    StatementProperties statement_properties = { false };
    return statement_properties;
}

static StatementProperties parseStructure(Parser* parser) {
    ASSERT_PARSER(parser);

    // Structure name
    forceMatch(parser, TOKEN_STRUCTURE);
    Token identifier_token = forceMatch(parser, TOKEN_IDENTIFIER);
    ValueType* structure_type = createStructureValueType(
        identifier_token.start,
        identifier_token.length
    );

    // Fields
    uint8_t reference_fields = 0;
    forceMatch(parser, TOKEN_LBRACE);
    while (!match(parser, TOKEN_RBRACE)) {
        
        // Field name and type
        Token field_identifier_token = forceMatch(parser, TOKEN_IDENTIFIER);
        forceMatch(parser, TOKEN_COLON);
        ValueType* field_type = parseValueType(parser);

        // If it's a reference type, save it's offset in a structure object.
        if (isReferenceValueType(field_type)) {
            pushOpCodeOnStack(parser->chunk, OP_PUSH_ADDRESS);
            pushAddressOnStack(parser->chunk, structure_type->as.structure.size);
            reference_fields += 1;
        }

        // Declare field in the structure.
        if (!addFieldToStructureValueType(
            structure_type,
            field_identifier_token.start,
            field_identifier_token.length,
            field_type
        )) {
            errorAt(
                parser,
                "Semantic",
                field_identifier_token,
                "Field %.*s redeclaration in structure %.*s.",
                field_identifier_token.length,
                field_identifier_token.start,
                identifier_token.length,
                identifier_token.start
            );
            break;
        }

        // // Comma
        // if (peekNext(parser) != TOKEN_RBRACE) {
        //     forceMatch(parser, TOKEN_COMMA);
        // }
    }

    // If it's a reference structure, push it onto the heap and onto the stack.
    if (reference_fields > 0) {
        pushOpCodeOnStack(parser->chunk, OP_DEFINE_ON_HEAP);
        pushAddressOnStack(parser->chunk, reference_fields * sizeof(size_t));
        pushByteOnStack(parser->chunk, REFERENCE_RULE_PLAIN);
    }

    // Declare structure in the symbol table.
    switch (declareVariableInScope(
        parser->scope,
        identifier_token.start,
        identifier_token.length,
        structure_type
    )) {
        case VARDECL_SUCCESS:
            break;

        case VARDECL_TOO_MANY_VARIABLES_IN_A_SCOPE:
            errorAt(
                parser,
                "Semantic",
                identifier_token,
                "Could not declare structure %.*s. "
                "Can't declare more than %d variables in a scope.",
                identifier_token.length,
                identifier_token.start,
                MAX_VARIABLES_IN_SCOPE
            );
            break;

        case VARDECL_VARIABLE_REDECLARATION:
            errorAt(
                parser,
                "Semantic",
                identifier_token,
                "Structure %.*s redeclares another variable.",
                identifier_token.length,
                identifier_token.start
            );
            break;

        default:
            assert(false);
    }

    ASSERT_PARSER(parser);
    StatementProperties statement_properties = { false };
    return statement_properties;
}

static ValueType* parseValueType(Parser* parser) {
    ASSERT_PARSER(parser);

    switch (advance(parser)) {
        case TOKEN_VOID:     return &VALUE_TYPE_VOID;
        case TOKEN_BOOL:     return &VALUE_TYPE_BOOL;
        case TOKEN_INT:      return &VALUE_TYPE_INT;
        case TOKEN_FLOAT:    return &VALUE_TYPE_FLOAT;
        case TOKEN_STRING:   return &VALUE_TYPE_STRING;
        
        case TOKEN_LBRACKET: {
            ValueType* element_type = parseValueType(parser);
            forceMatch(parser, TOKEN_RBRACKET);
            return createArrayValueType(element_type);
        }
        
        case TOKEN_IDENTIFIER: {
            // Find the structure variable.
            Variable variable;
            bool found_variable = accessVariableInScope(
                parser->scope,
                previous(parser).start,
                previous(parser).length,
                &variable
            );

            // Make sure the variable is present.
            if (!found_variable) {
                errorAtPrevious(
                    parser,
                    "Semantic",
                    "Type %.*s isn't declared.",
                    previous(parser).length,
                    previous(parser).start
                );
                return &VALUE_TYPE_INVALID;
            }

            // Make sure the variable is a structure.
            if (!isStructureValueType(variable.type)) {
                errorAtPrevious(
                    parser,
                    "Semantic",
                    "Expected a structure in type specifier, got a %s.",
                    valueTypeName(variable.type)
                );
            }

            return variable.type->as.structure.instance_type;
        }

        default:
            errorAtPrevious(
                parser,
                "Syntactic",
                "Expected type specifier: bool, int, float, string or array. Got %s.",
                tokenTypeName(previous(parser).type)
            );
            return &VALUE_TYPE_INVALID;
    }
}

static StatementProperties parseStatement(Parser* parser) {
    ASSERT_PARSER(parser);

    switch (peekNext(parser)) {
        case TOKEN_IDENTIFIER: {
            parsePostfix(parser, EXPRESSION_STATEMENT);
            StatementProperties statement_properties = { false };
            return statement_properties;
        }

        case TOKEN_PRINT:      return parsePrint(parser);
        case TOKEN_IF:         return parseIf(parser);
        case TOKEN_WHILE:      return parseWhile(parser);
        case TOKEN_DO:         return parseDoWhile(parser);
        case TOKEN_CONTINUE:   return parseContinue(parser);
        case TOKEN_BREAK:      return parseBreak(parser);
        case TOKEN_RETURN:     return parseReturn(parser);
        case TOKEN_LBRACE:     return parseBlock(parser);

        default: {
            errorAtNext(
                parser,
                "Syntactic",
                "Unexpected token on statement start. Expected TOKEN_PRINT, TOKEN_IDENTIFIER, TOKEN_IF, TOKEN_WHILE, TOKEN_DO, got %s",
                tokenTypeName(peekNext(parser))
            );
            StatementProperties statement_properties = { false };
            return statement_properties;
        }
    }
}

static StatementProperties parsePrint(Parser* parser) {
    ASSERT_PARSER(parser);

    forceMatch(parser, TOKEN_PRINT);
    Token expression_start_token = next(parser);
    ValueType* value_type = parseExpression(parser);
    switch (value_type->basic_type) {
        case BASIC_VALUE_TYPE_BOOL:   pushOpCodeOnStack(parser->chunk, OP_PRINT_BOOL);   break;
        case BASIC_VALUE_TYPE_INT:    pushOpCodeOnStack(parser->chunk, OP_PRINT_INT);    break;
        case BASIC_VALUE_TYPE_FLOAT:  pushOpCodeOnStack(parser->chunk, OP_PRINT_FLOAT);  break;
        case BASIC_VALUE_TYPE_STRING: pushOpCodeOnStack(parser->chunk, OP_PRINT_STRING); break;
        default:
            error(
                parser,
                "Semantic",
                expression_start_token,
                previous(parser),
                "Print statement supports BOOL, INT, FLOAT, STRING arguments, got %s",
                valueTypeName(value_type)
            );
            break;
    }

    ASSERT_PARSER(parser);
    StatementProperties statement_properties = { false };
    return statement_properties;
}

static StatementProperties parseIf(Parser* parser){
    ASSERT_PARSER(parser);

    forceMatch(parser, TOKEN_IF);
    
    // Parse condition expression
    Token expression_start_token = next(parser);
    ValueType* condition_value_type = parseExpression(parser);

    // Make sure condition is bool
    if (condition_value_type->basic_type != BASIC_VALUE_TYPE_BOOL) {
        error(
            parser,
            "Semantic",
            expression_start_token,
            previous(parser),
            "Condition expression in an if statement is %s, but has to be bool.",
            valueTypeName(condition_value_type)
        );
        StatementProperties statement_properties = { false };
        return statement_properties;
    }

    // Jump over the body if condition is false; fill jump address later
    pushOpCodeOnStack(parser->chunk, OP_JUMP_IF_FALSE);
    size_t after_if_address_position_in_chunk = stackSize(parser->chunk);
    pushAddressOnStack(parser->chunk, (size_t)0);

    // Parse if body
    StatementProperties statement_properties = { false };
    StatementProperties if_body_properties = parseStatement(parser);

    // Fill jump over the body address
    size_t after_if_address = stackSize(parser->chunk);
    setAddressOnStack(parser->chunk, after_if_address_position_in_chunk, after_if_address);

    // Parse else body if present
    if (match(parser, TOKEN_ELSE)) {
        StatementProperties else_body_properties = parseStatement(parser);
        statement_properties.ends_with_return = (
            if_body_properties.ends_with_return && 
            else_body_properties.ends_with_return
        );
    }

    ASSERT_PARSER(parser);
    return statement_properties;
}

static StatementProperties parseWhile(Parser* parser){
    ASSERT_PARSER(parser);

    forceMatch(parser, TOKEN_WHILE);

    // Save iteration start location
    size_t iteration_start_address = stackSize(parser->chunk);

    // Parse condition expression
    Token expression_start_token = next(parser);
    ValueType* condition_value_type = parseExpression(parser);

    // Make sure condition is bool
    if (condition_value_type->basic_type != BASIC_VALUE_TYPE_BOOL) {
        error(
            parser,
            "Semantic",
            expression_start_token,
            previous(parser),
            "Condition expression in an if statement is %s, but has to be bool.",
            valueTypeName(condition_value_type)
        );
        StatementProperties statement_properties = { false };
        return statement_properties;
    }

    // Jump out of while if condition is false; fill jump address later
    pushOpCodeOnStack(parser->chunk, OP_JUMP_IF_FALSE);
    size_t after_while_address_position_in_chunk = stackSize(parser->chunk);
    pushAddressOnStack(parser->chunk, (size_t)0);

    // Parse body
    parseStatement(parser);

    // Jump to the start of while statement after an iteration
    pushOpCodeOnStack(parser->chunk, OP_JUMP);
    pushAddressOnStack(parser->chunk, iteration_start_address);

    // Fill jump out address
    size_t after_while_address = stackSize(parser->chunk);
    setAddressOnStack(parser->chunk, after_while_address_position_in_chunk, after_while_address);

    ASSERT_PARSER(parser);
    StatementProperties statement_properties = { false };
    return statement_properties;
}

static StatementProperties parseDoWhile(Parser* parser){
    ASSERT_PARSER(parser);

    forceMatch(parser, TOKEN_DO);

    // Save iteration start location
    size_t iteration_start_address = stackSize(parser->chunk);

    // Parse body
    StatementProperties statement_properties = parseStatement(parser);

    forceMatch(parser, TOKEN_WHILE);

    // Parse condition expression
    Token expression_start_token = next(parser);
    ValueType* condition_value_type = parseExpression(parser);

    // Make sure condition is false
    if (condition_value_type->basic_type != BASIC_VALUE_TYPE_BOOL) {
        error(
            parser,
            "Semantic",
            expression_start_token,
            previous(parser),
            "Condition expression in an if statement is %s, but has to be bool.",
            valueTypeName(condition_value_type)
        );
    }

    // Jump to the start of do-while statement after an iteration, if the condition is true
    pushOpCodeOnStack(parser->chunk, OP_JUMP_IF_TRUE);
    pushAddressOnStack(parser->chunk, iteration_start_address);

    ASSERT_PARSER(parser);
    return statement_properties;
}

static StatementProperties parseContinue(Parser* parser) {
    ASSERT_PARSER(parser);

    forceMatch(parser, TOKEN_CONTINUE);
    errorAtPrevious(parser, "Syntax", "Continue isn't implemented yet.");

    ASSERT_PARSER(parser);
    StatementProperties statement_properties = { false };
    return statement_properties;
}

static StatementProperties parseBreak(Parser* parser) {
    ASSERT_PARSER(parser);

    forceMatch(parser, TOKEN_BREAK);
    errorAtPrevious(parser, "Syntax", "Break isn't implemented yet.");

    ASSERT_PARSER(parser);
    StatementProperties statement_properties = { false };
    return statement_properties;
}

static StatementProperties parseReturn(Parser* parser) {
    ASSERT_PARSER(parser);

    forceMatch(parser, TOKEN_RETURN);

    // Make sure the return statement is inside a function.
    ValueType* expected_return_type = getReturnType(parser->scope);
    if (!expected_return_type) {
        errorAtPrevious(parser, "Semantic", "Return statement outside of a function.");
    }

    // Return value
    if (expected_return_type != &VALUE_TYPE_VOID) {
        Token expression_start_token = next(parser);
        ValueType* return_expression_type = parseExpression(parser);
        
        if (parser->panic_mode) {
            ASSERT_PARSER(parser);
            StatementProperties statement_properties = { true };
            return statement_properties;
        }

        // Make sure return value's type corresponds with the function return type.
        if (!valueTypesEqual(expected_return_type, return_expression_type)) {
            error(
                parser,
                "Semantic",
                expression_start_token,
                previous(parser),
                "Return value is a %s, but the function is expected to return a %s.",
                valueTypeName(return_expression_type),
                valueTypeName(expected_return_type)
            );
        }
    }

    // OP_RETURN
    pushOpCodeOnStack(parser->chunk, getOpReturnForValueType(expected_return_type));

    ASSERT_PARSER(parser);
    StatementProperties statement_properties = { true };
    return statement_properties;
}

static StatementProperties parseBlock(Parser* parser) {
    ASSERT_PARSER(parser);

    StatementProperties statement_properties = { false };

    // Enter a new scope.
    forceMatch(parser, TOKEN_LBRACE);
    parser->scope = createScope(parser->scope);

    // Parse block statements.
    while (!match(parser, TOKEN_RBRACE) && peekNext(parser) != TOKEN_END) {
        StatementProperties current_declaration_properties = parseDeclaration(parser);
        // If the statement always ends with return.
        if (current_declaration_properties.ends_with_return) {
            statement_properties.ends_with_return = true;
            // Make sure there're no more statements in the block.
            if (peekNext(parser) != TOKEN_RBRACE) {
                errorAtNext(
                    parser,
                    "Semantic",
                    "Statement is unreachable."
                );
                break;
            }
        }
    }

    // Exit the block scope.
    assert(parser->scope->parent);
    size_t block_scope_locals_size = (
        parser->scope->stack_top - parser->scope->parent->stack_top
    );
    pushOpCodeOnStack(parser->chunk, OP_POP_BYTES);
    pushAddressOnStack(parser->chunk, block_scope_locals_size);

    parser->scope = deleteScope(parser->scope);
    assert(parser->scope);

    ASSERT_PARSER(parser);
    return statement_properties;
}

static ValueType* parseOr(Parser* parser) {
    ASSERT_PARSER(parser);

    Token expression_start_token = next(parser);
    ValueType* value_type_l = parseAnd(parser);

    while (match(parser, TOKEN_OR)) {
        ValueType* value_type_r = parseAnd(parser);

        validateOperatorTypes(parser, expression_start_token, TOKEN_OR, value_type_l->basic_type, value_type_r->basic_type);
        emitOpCodesForTokenAndValueTypesCombination(parser, 2, TOKEN_OR, value_type_l->basic_type);
    }

    ASSERT_PARSER(parser);
    assert(value_type_l);
    return value_type_l;
}

static ValueType* parseAnd(Parser* parser) {
    ASSERT_PARSER(parser);

    Token expression_start_token = next(parser);
    ValueType* value_type_l = parseComparison(parser);
    
    while (match(parser, TOKEN_AND)) {
        ValueType* value_type_r = parseComparison(parser);

        validateOperatorTypes(parser, expression_start_token, TOKEN_AND, value_type_l->basic_type, value_type_r->basic_type);
        emitOpCodesForTokenAndValueTypesCombination(parser, 2, TOKEN_AND, value_type_l->basic_type);
    }

    ASSERT_PARSER(parser);
    assert(value_type_l);
    return value_type_l;
}

static ValueType* parseComparison(Parser* parser) {
    ASSERT_PARSER(parser);

    Token expression_start_token = next(parser);
    ValueType* value_type_l = parseTerm(parser);
    
    if (match(parser, TOKEN_EQUAL_EQUAL)       ||
        match(parser, TOKEN_EXCLAMATION_EQUAL) ||
        match(parser, TOKEN_GREATER_EQUAL)     ||
        match(parser, TOKEN_LESS_EQUAL)        ||
        match(parser, TOKEN_GREATER)           ||
        match(parser, TOKEN_LESS)
    ) {
        TokenType operator_token_type = previous(parser).type;
        ValueType* value_type_r = parseTerm(parser);

        validateOperatorTypes(parser, expression_start_token, operator_token_type, value_type_l->basic_type, value_type_r->basic_type);
        emitOpCodesForTokenAndValueTypesCombination(parser, 2, operator_token_type, value_type_l->basic_type);

        value_type_l = &VALUE_TYPE_BOOL;
    }

    ASSERT_PARSER(parser);
    assert(value_type_l);
    return value_type_l;
}

static ValueType* parseTerm(Parser* parser) {
    ASSERT_PARSER(parser);

    Token expression_start_token = next(parser);
    ValueType* value_type_l = parseFactor(parser);
    
    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS)) {
        TokenType operator_token_type = previous(parser).type;
        ValueType* value_type_r = parseFactor(parser);

        validateOperatorTypes(parser, expression_start_token, operator_token_type, value_type_l->basic_type, value_type_r->basic_type);
        emitOpCodesForTokenAndValueTypesCombination(parser, 2, operator_token_type, value_type_l->basic_type);
    }

    ASSERT_PARSER(parser);
    assert(value_type_l);
    return value_type_l;
}

static ValueType* parseFactor(Parser* parser) {
    ASSERT_PARSER(parser);

    Token expression_start_token = next(parser);
    ValueType* value_type_l = parsePrefix(parser);
    
    while (
        match(parser, TOKEN_STAR)    ||
        match(parser, TOKEN_SLASH)   || 
        match(parser, TOKEN_PERCENT)
    ) {
        TokenType operator_token_type = previous(parser).type;
        ValueType* value_type_r = parsePrefix(parser);

        validateOperatorTypes(parser, expression_start_token, operator_token_type, value_type_l->basic_type, value_type_r->basic_type);
        emitOpCodesForTokenAndValueTypesCombination(parser, 2, operator_token_type, value_type_l->basic_type);
    }

    ASSERT_PARSER(parser);
    assert(value_type_l);
    return value_type_l;
}

static ValueType* parsePrefix(Parser* parser) {
    ASSERT_PARSER(parser);

    bool had_prefix_operator = false;
    TokenType operator_token_type;
    if (match(parser, TOKEN_MINUS) || match(parser, TOKEN_EXCLAMATION)) {
        had_prefix_operator = true;
        operator_token_type = previous(parser).type;
    }
    
    ValueType* value_type = parsePostfix(parser, EXPRESSION);

    if (had_prefix_operator) {
        if (operator_token_type == TOKEN_MINUS) {
            BasicValueType expected[4] = { BASIC_VALUE_TYPE_INT, BASIC_VALUE_TYPE_FLOAT };
            validateOperandType(value_type->basic_type, expected);
        } else if (operator_token_type == TOKEN_EXCLAMATION) {
            BasicValueType expected[4] = { BASIC_VALUE_TYPE_BOOL };
            validateOperandType(value_type->basic_type, expected);
        }
        emitOpCodesForTokenAndValueTypesCombination(parser, 1, operator_token_type, value_type->basic_type);
    }

    ASSERT_PARSER(parser);
    assert(value_type);
    return value_type;
}

static ValueType* parsePostfix(Parser* parser, ExpressionKind expression_kind) {
    ASSERT_PARSER(parser);

    ValueType* value_type = parsePrimary(parser, expression_kind);

    // If the primary was an assignment to a variable, return.
    if (expression_kind == EXPRESSION_STATEMENT && value_type == NULL) {
        ASSERT_PARSER(parser);
        return NULL;
    }

    Token last_postfix_op_token = previous(parser);

    while (
        match(parser, TOKEN_DOT)      ||
        match(parser, TOKEN_LPAREN)   ||
        match(parser, TOKEN_LBRACKET) ||
        match(parser, TOKEN_COLON)
    ) {
        last_postfix_op_token = previous(parser);
        switch (previous(parser).type) {
            
            // Member access
            case TOKEN_DOT: {
                // Make sure the value is an object.
                if (value_type->basic_type != BASIC_VALUE_TYPE_OBJECT) {
                    errorAtPrevious(
                        parser,
                        "Semantic",
                        "Trying to access a member of a %s. Only objects have members.",
                        valueTypeName(value_type)
                    )
                    return &VALUE_TYPE_INVALID;
                }

                StructureValueType structure = value_type->as.object.structure_type->as.structure;

                Token member_identifier_token = forceMatch(parser, TOKEN_IDENTIFIER);
                
                size_t field_index;
                if (!getFromHashMap(
                    &structure.fields_map,
                    member_identifier_token.start,
                    member_identifier_token.length,
                    &field_index
                )) {
                    errorAtPrevious(
                        parser,
                        "Semantic",
                        "Field %.*s doesn't exist.",
                        member_identifier_token.length,
                        member_identifier_token.start
                    );
                }
                assert(field_index < structure.fields_map.count);
                Field field = structure.fields_properties[field_index];

                OpCode op_code = getOpGetFromHeapForValueType(field.type);

                // If it's an expression statement and this postfix is the last postfix in the lhs,
                // parse the assignment.
                if (expression_kind == EXPRESSION_STATEMENT && match(parser, TOKEN_EQUAL)) {
                    // The assignment rhs.
                    Token expression_start_token = next(parser);
                    ValueType* expression_value_type = parseExpression(parser);

                    // Make sure the field and value types match.
                    if (!valueTypesEqual(field.type, expression_value_type)) {
                        error(
                            parser,
                            "Semantic",
                            expression_start_token,
                            previous(parser),
                            "Field type (%s) and expression type (%s) don't match in an assignment.",
                            valueTypeName(field.type),
                            valueTypeName(expression_value_type)
                        );
                    }

                    // Replace get opcode with set opcode.
                    op_code = getOpSetOnHeapForValueType(field.type);
                    value_type = NULL;
                } else {
                    value_type = field.type;
                }

                // Get or set the field.
                pushOpCodeOnStack(parser->chunk, op_code);
                pushAddressOnStack(parser->chunk, field.offset);

                break;
            }

            // Call
            case TOKEN_LPAREN: {
                // Make sure the value is a function.
                if (value_type->basic_type != BASIC_VALUE_TYPE_FUNCTION) {
                    errorAtPrevious(
                        parser,
                        "Semantic",
                        "Trying to call a %s. Only functions may be called.",
                        valueTypeName(value_type))
                    return &VALUE_TYPE_INVALID;
                }

                FunctionValueType function = value_type->as.function;

                // Return address; will be filled later.
                pushOpCodeOnStack(parser->chunk, OP_PUSH_ADDRESS);
                size_t return_address_position_in_chunk = stackSize(parser->chunk);
                pushAddressOnStack(parser->chunk, (size_t)0);

                // Arguments.
                for (uint8_t i = 0; i < function.arity; ++i) {
                    // Make sure the arguments list isn't over.
                    if (peekNext(parser) == TOKEN_RPAREN) {
                        errorAtNext(
                            parser,
                            "Semantic",
                            "Expected the next argument %s.",
                            valueTypeName(function.parameter_types[i])
                        );
                    }

                    // Argument
                    Token argument_expression_start_token = next(parser);
                    ValueType* argument_type = parseExpression(parser);

                    // Make sure the argument type matches the parameter type.
                    if (!valueTypesEqual(function.parameter_types[i], argument_type)) {
                        error(
                            parser,
                            "Semantic",
                            argument_expression_start_token,
                            previous(parser),
                            "Argument type %s doesn't match parameter type %s.",
                            valueTypeName(argument_type),
                            valueTypeName(function.parameter_types[i])
                        );
                    }

                    // Make sure all but the last arguments are followed by a comma.
                    // The last argument may optionally be followed by a comma.
                    if (!match(parser, TOKEN_COMMA) && i < function.arity - 1) {
                        errorAtNext(
                            parser,
                            "Syntactic",
                            "Expected a comma and the next argument %s.",
                            valueTypeName(function.parameter_types[i + 1])
                        );
                    }
                }
                forceMatch(parser, TOKEN_RPAREN);

                // Call op.
                pushOpCodeOnStack(parser->chunk, OP_CALL);
                // On CALL invocation, the top of the stack should be as follows:
                //   - function address –– sizeof(size_t)
                //   - return address   –– sizeof(size_t)
                //   - arguments        –– function.parameters_size
                size_t offset_from_call_frame_start = 2 * sizeof(size_t) + function.parameters_size;
                pushAddressOnStack(parser->chunk, offset_from_call_frame_start);

                // Fill the return address.
                size_t return_address = stackSize(parser->chunk);
                setAddressOnStack(parser->chunk, return_address_position_in_chunk, return_address);

                // Remove the return value in an expression statement if it's the last postfix op.
                if (
                    expression_kind == EXPRESSION_STATEMENT &&
                    peekNext(parser) != TOKEN_DOT           &&
                    peekNext(parser) != TOKEN_LPAREN        &&
                    peekNext(parser) != TOKEN_LBRACKET      &&
                    peekNext(parser) != TOKEN_COLON
                ) {
                    pushOpCodeOnStack(parser->chunk, getOpPopForValueType(function.return_type));
                }

                value_type = function.return_type;
                break;
            }
            
            // Subscript
            case TOKEN_LBRACKET: {
                // Make sure the value is array.
                if (value_type->basic_type != BASIC_VALUE_TYPE_ARRAY) {
                    errorAtPrevious(
                        parser,
                        "Semantic",
                        "Trying to subscript a %s. Only arrays may be subscripted.",
                        valueTypeName(value_type)
                    );
                    return &VALUE_TYPE_INVALID;
                }

                // Index expression
                Token expression_start_token = next(parser);
                ValueType* index_type = parseExpression(parser);
                forceMatch(parser, TOKEN_RBRACKET);

                // Make sure index is an int.
                if (index_type->basic_type != BASIC_VALUE_TYPE_INT) {
                    error(
                        parser,
                        "Semantic",
                        expression_start_token,
                        previous(parser),
                        "Invalid array index type %s. Only int can be an array index.",
                        valueTypeName(index_type)
                    );
                    return &VALUE_TYPE_INVALID;
                }

                OpCode op_code = getOpSubscriptGetForValueType(value_type->as.array.element_type);

                // If it's an expression statement and this postfix is the last postfix in the lhs,
                // parse the assignment.
                if (expression_kind == EXPRESSION_STATEMENT && match(parser, TOKEN_EQUAL)) {
                    // The assignment rhs.
                    Token expression_start_token = next(parser);
                    ValueType* expression_value_type = parseExpression(parser);

                    // Make sure the array elements and value types match.
                    if (!valueTypesEqual(value_type->as.array.element_type, expression_value_type)) {
                        error(
                            parser,
                            "Semantic",
                            expression_start_token,
                            previous(parser),
                            "Array element type (%s) and expression type (%s) don't match in an assignment.",
                            valueTypeName(value_type->as.array.element_type),
                            valueTypeName(expression_value_type)
                        );
                    }

                    // Replace get opcode with set opcode.
                    op_code = getOpSubscriptSetForValueType(value_type->as.array.element_type);
                    value_type = NULL;
                } else {
                    value_type = value_type->as.array.element_type;
                }

                // Get or set the array element.
                pushOpCodeOnStack(parser->chunk, op_code);

                break;
            }

            // Type cast
            case TOKEN_COLON:
                switch (advance(parser)) {
                    
                    // Cast float to int
                    case TOKEN_INT: {
                        BasicValueType expected[4] = { BASIC_VALUE_TYPE_FLOAT };
                        validateOperandType(value_type->basic_type, expected);
                        pushOpCodeOnStack(parser->chunk, OP_CAST_FLOAT_TO_INT);
                        value_type = &VALUE_TYPE_INT;
                        break;
                    }
                    
                    // Cast int to float
                    case TOKEN_FLOAT: {
                        BasicValueType expected[4] = { BASIC_VALUE_TYPE_INT };
                        validateOperandType(value_type->basic_type, expected);
                        pushOpCodeOnStack(parser->chunk, OP_CAST_INT_TO_FLOAT);
                        value_type = &VALUE_TYPE_FLOAT;
                        break;
                    }
                    
                    // Cast to string
                    case TOKEN_STRING: {
                        BasicValueType expected[4] = { BASIC_VALUE_TYPE_BOOL, BASIC_VALUE_TYPE_INT, BASIC_VALUE_TYPE_FLOAT };
                        validateOperandType(value_type->basic_type, expected);

                        switch (value_type->basic_type) {
                            case BASIC_VALUE_TYPE_BOOL:
                                pushOpCodeOnStack(parser->chunk, OP_CAST_BOOL_TO_STRING);
                                break;
                            case BASIC_VALUE_TYPE_INT:
                                pushOpCodeOnStack(parser->chunk, OP_CAST_INT_TO_STRING);
                                break;
                            case BASIC_VALUE_TYPE_FLOAT:
                                pushOpCodeOnStack(parser->chunk, OP_CAST_FLOAT_TO_STRING);
                                break;
                            default:
                                assert(false);
                        }

                        value_type = &VALUE_TYPE_STRING;
                        break;
                    }

                    // Invalid cast
                    default:
                        errorAtPrevious(
                            parser,
                            "Syntactic",
                            "Unexpected token for type cast: %s.",
                            tokenTypeName(previous(parser).type)
                        );
                        return &VALUE_TYPE_INVALID;
                }
                break;

            default:
                assert(false);
        }
    }

    // Make sure expression statement ended with either an assignment or a call.
    if (expression_kind == EXPRESSION_STATEMENT) {
        switch (last_postfix_op_token.type) {
            case TOKEN_DOT:
            case TOKEN_LPAREN:
            case TOKEN_LBRACKET:
                // Correct expression statement.
                break;

            case TOKEN_COLON:
                errorAt(
                    parser,
                    "Syntax",
                    last_postfix_op_token,
                    "Expression statement can't end with a type cast. "
                    "Expression statement may end with either an assignment or a call."
                );
                break;

            default:
                errorAt(
                    parser,
                    "Syntax",
                    last_postfix_op_token,
                    "Expected an expression statement. "
                    "Expression statement may end with either an assignment or a call."
                );
                break;
        }
    }

    switch (expression_kind) {
        case EXPRESSION:
            ASSERT_PARSER(parser);
            assert(value_type);
            return value_type;

        case EXPRESSION_STATEMENT:
            ASSERT_PARSER(parser);
            return NULL;

        default:
            assert(false);
    }
}

static ValueType* parsePrimary(Parser* parser, ExpressionKind expression_kind) {
    ASSERT_PARSER(parser);

    ValueType* value_type;

    if (expression_kind == EXPRESSION_STATEMENT) {
        assert(peekNext(parser) == TOKEN_IDENTIFIER);
    }

    switch (advance(parser)) {

        case TOKEN_TRUE:
            pushOpCodeOnStack(parser->chunk, OP_PUSH_TRUE);
            value_type = &VALUE_TYPE_BOOL;
            break;
        
        case TOKEN_FALSE:
            pushOpCodeOnStack(parser->chunk, OP_PUSH_FALSE);
            value_type = &VALUE_TYPE_BOOL;
            break;
        
        case TOKEN_INTEGER_VALUE: {
            int32_t value = (int32_t)strtol(previous(parser).start, NULL, 10);
            pushOpCodeOnStack(parser->chunk, OP_PUSH_INT);
            pushIntOnStack(parser->chunk, value);
            value_type = &VALUE_TYPE_INT;
            break;
        }
        
        case TOKEN_FLOAT_VALUE: {
            double value = strtod(previous(parser).start, NULL);
            pushOpCodeOnStack(parser->chunk, OP_PUSH_FLOAT);
            pushFloatOnStack(parser->chunk, value);
            value_type = &VALUE_TYPE_FLOAT;
            break;
        }
        
        case TOKEN_STRING_VALUE: {
            uint8_t string_constant_i = addConstant(
                &parser->constants,
                previous(parser).length - 2,
                (const uint8_t*)previous(parser).start + 1
            );
            pushOpCodeOnStack(parser->chunk, OP_LOAD_CONSTANT);
            pushByteOnStack(parser->chunk, string_constant_i);
            value_type = &VALUE_TYPE_STRING;
            break;
        }

        case TOKEN_READ:
            switch (advance(parser)) {
                case TOKEN_BOOL:
                    pushOpCodeOnStack(parser->chunk, OP_READ_BOOL);
                    value_type = &VALUE_TYPE_BOOL;
                    break;
                case TOKEN_INT:
                    pushOpCodeOnStack(parser->chunk, OP_READ_INT);
                    value_type = &VALUE_TYPE_INT;
                    break;
                case TOKEN_FLOAT:
                    pushOpCodeOnStack(parser->chunk, OP_READ_FLOAT);
                    value_type = &VALUE_TYPE_FLOAT;
                    break;
                case TOKEN_STRING:
                    pushOpCodeOnStack(parser->chunk, OP_READ_STRING);
                    value_type = &VALUE_TYPE_STRING;
                    break;
                default:
                    errorAtPrevious(
                        parser,
                        "Semantic",
                        "Expected read to be followed by bool, int, float or string. Got %s.",
                        tokenTypeName(previous(parser).type)
                    );
                    break;
            }
            break;
        
        case TOKEN_IDENTIFIER: {
            // Find the variable.
            Variable variable;
            bool found_variable = accessVariableInScope(
                parser->scope,
                previous(parser).start,
                previous(parser).length,
                &variable
            );

            // Make sure the variable is present.
            if (!found_variable) {
                errorAtPrevious(
                    parser,
                    "Semantic",
                    "Access to undeclared variable %.*s.",
                    previous(parser).length,
                    previous(parser).start
                );
                return &VALUE_TYPE_INVALID;
            }

            // Structure object instantiation.
            if (isStructureValueType(variable.type)) {
                StructureValueType structure = variable.type->as.structure;
                size_t fields_count = structure.fields_map.count;

                forceMatch(parser, TOKEN_LPAREN);

                // Arguments
                for (size_t i = 0; i < fields_count; ++i) {
                    // Make sure the arguments list isn't over.
                    if (peekNext(parser) == TOKEN_RPAREN) {
                        errorAtNext(
                            parser,
                            "Semantic",
                            "Expected the next %s argument.",
                            valueTypeName(structure.fields_properties[i].type)
                        );
                    }

                    // Argument
                    Token argument_expression_start_token = next(parser);
                    ValueType* argument_type = parseExpression(parser);

                    // Make sure the argument type matches the field type.
                    if (!valueTypesEqual(structure.fields_properties[i].type, argument_type)) {
                        error(
                            parser,
                            "Semantic",
                            argument_expression_start_token,
                            previous(parser),
                            "Argument type %s doesn't match field type %s.",
                            valueTypeName(argument_type),
                            valueTypeName(structure.fields_properties[i].type)
                        );
                    }

                    // Make sure all but the last arguments are followed by a comma.
                    // The last argument may optionally be followed by a comma.
                    if (!match(parser, TOKEN_COMMA) && i < fields_count - 1) {
                        errorAtNext(
                            parser,
                            "Syntactic",
                            "Expected a comma and the next argument %s.",
                            valueTypeName(structure.fields_properties[i].type)
                        );
                    }
                }
                forceMatch(parser, TOKEN_RPAREN);

                // If it's a reference structure, get the structure definition from the stack.
                if (variable.type->basic_type == BASIC_VALUE_TYPE_REFERENCE_STRUCTURE) {
                    pushOpCodeOnStack(
                        parser->chunk,
                        getOpGetFromStackForValueType(variable.type, variable.kind)
                    );
                    pushAddressOnStack(parser->chunk, variable.address_on_stack);
                }

                // Instantiate the structure.
                pushOpCodeOnStack(parser->chunk, OP_DEFINE_ON_HEAP);
                pushAddressOnStack(parser->chunk, structure.size);
                pushByteOnStack(
                    parser->chunk,
                    (
                        variable.type->basic_type == BASIC_VALUE_TYPE_PLAIN_STRUCTURE ?
                        REFERENCE_RULE_PLAIN :
                        REFERENCE_RULE_CUSTOM
                    )
                );

                value_type = structure.instance_type;
            }

            // Variable get/set.
            else {
                // Get variable from stack opcode.
                OpCode op_code = getOpGetFromStackForValueType(variable.type, variable.kind);

                // If it's an expression statement and the identifier is an assignment target,
                // parse the assignment and replace the get opcode with set opcdode.
                if (expression_kind == EXPRESSION_STATEMENT && match(parser, TOKEN_EQUAL)) {
                    // The assignment rhs.
                    Token expression_start_token = next(parser);
                    ValueType* expression_value_type = parseExpression(parser);

                    // Make sure the variable and value types match.
                    if (!valueTypesEqual(variable.type, expression_value_type)) {
                        error(
                            parser,
                            "Semantic",
                            expression_start_token,
                            previous(parser),
                            "Variable type (%s) and expression type (%s) don't match in an assignment.",
                            valueTypeName(variable.type),
                            valueTypeName(expression_value_type)
                        );
                    }

                    // Replace get opcode with set opcode.
                    op_code = getOpSetOnStackForValueType(variable.type, variable.kind);
                    value_type = NULL;
                } else {
                    value_type = variable.type;
                }

                // Get or set the variable.
                pushOpCodeOnStack(parser->chunk, op_code);
                pushAddressOnStack(parser->chunk, variable.address_on_stack);
            }

            break;
        }
        
        case TOKEN_LBRACKET: {
            ValueType* element_type = NULL;
            size_t elements_count = 0;

            // TODO: produce an error message in case of unterminated array; same with blocks
            
            while (!match(parser, TOKEN_RBRACKET) && peekNext(parser) != TOKEN_END) {
                Token expression_start_token = next(parser);
                ValueType* current_element_type = parseExpression(parser);
                if (current_element_type == &VALUE_TYPE_INVALID) {
                    return &VALUE_TYPE_INVALID;
                }

                if (element_type == NULL) {
                    element_type = current_element_type;
                }

                if (!valueTypesEqual(current_element_type, element_type)) {
                    error(
                        parser,
                        "Semantic",
                        expression_start_token,
                        previous(parser),
                        "Invalid array element type %s in an array of %s",
                        valueTypeName(current_element_type),
                        valueTypeName(element_type)
                    );
                    return &VALUE_TYPE_INVALID;
                }

                if (peekNext(parser) != TOKEN_RBRACKET) {
                    forceMatch(parser, TOKEN_COMMA);
                }

                elements_count += 1;
            }
        
            pushOpCodeOnStack(parser->chunk, OP_DEFINE_ON_HEAP);
            pushAddressOnStack(parser->chunk, elements_count * valueTypeSize(element_type));
            pushByteOnStack(
                parser->chunk,
                isReferenceValueType(element_type) ? REFERENCE_RULE_REF_ARRAY : REFERENCE_RULE_PLAIN
            );

            value_type = createArrayValueType(element_type);
            break;
        }
        
        case TOKEN_LBRACE:
            errorAtPrevious(
                parser,
                "Syntactic",
                "Maps not implemented yet."
            );
            return &VALUE_TYPE_INVALID;
        
        case TOKEN_LPAREN:
            value_type = parseExpression(parser);
            forceMatch(parser, TOKEN_RPAREN);
            break;
        
        default:
            errorAtPrevious(
                parser,
                "Syntactic",
                "Unexpected token %s while parsing a primary value.",
                tokenTypeName(previous(parser).type)
            );
            return &VALUE_TYPE_INVALID;
    }

    ASSERT_PARSER(parser);
    if (expression_kind == EXPRESSION) {
        assert(value_type);
    }
    return value_type;
}


// ─────────────────────
//  Operator type rules 
// ─────────────────────

typedef struct {
    BasicValueType  left_operand_expected_types[4];
    BasicValueType right_operand_expected_types[4];
    bool types_have_to_match;
} OperatorTypeRules;

#define B    { BASIC_VALUE_TYPE_BOOL }
#define I    { BASIC_VALUE_TYPE_INT }
#define IF   { BASIC_VALUE_TYPE_INT,  BASIC_VALUE_TYPE_FLOAT }
#define IFS  { BASIC_VALUE_TYPE_INT,  BASIC_VALUE_TYPE_FLOAT, BASIC_VALUE_TYPE_STRING }
#define BIFS { BASIC_VALUE_TYPE_BOOL, BASIC_VALUE_TYPE_INT,   BASIC_VALUE_TYPE_FLOAT, BASIC_VALUE_TYPE_STRING }
#define A    { BASIC_VALUE_TYPE_ARRAY }

OperatorTypeRules operator_type_rules[] = {
    [TOKEN_OR]                = { B,     B,     true  },
    [TOKEN_AND]               = { B,     B,     true  },
    [TOKEN_EQUAL_EQUAL]       = { BIFS,  BIFS,  true  },
    [TOKEN_EXCLAMATION_EQUAL] = { BIFS,  BIFS,  true  },
    [TOKEN_GREATER_EQUAL]     = { IFS,   IFS,   true  },
    [TOKEN_LESS_EQUAL]        = { IFS,   IFS,   true  },
    [TOKEN_GREATER]           = { IFS,   IFS,   true  },
    [TOKEN_LESS]              = { IFS,   IFS,   true  },
    [TOKEN_PLUS]              = { IFS,   IFS,   true  },
    [TOKEN_MINUS]             = { IF,    IF,    true  },
    [TOKEN_STAR]              = { IF,    IF,    true  },
    [TOKEN_SLASH]             = { IF,    IF,    true  },
    [TOKEN_PERCENT]           = { IF,    I,     false },
    [TOKEN_EXCLAMATION]       = { B,     B,     true  },
    [TOKEN_LBRACKET]          = { A,     I,     false },  // Subscript
};

#undef BIFS
#undef IFS
#undef IF
#undef I
#undef B

static bool validateOperandType(
    BasicValueType operand_type,
    BasicValueType expected_operand_types[4]
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

static void validateOperatorTypes(
    Parser* parser,
    Token expression_start_token,
    TokenType operator,
    BasicValueType left_operand_type,
    BasicValueType right_operand_type
) {
    ASSERT_PARSER(parser);

    OperatorTypeRules rules = operator_type_rules[operator];

    if (!validateOperandType(left_operand_type,  rules.left_operand_expected_types) ||
        !validateOperandType(right_operand_type, rules.right_operand_expected_types)
    ) {
        error(
            parser,
            "Semantic",
            expression_start_token,
            previous(parser),
            "One of the operands has an invalid type for operator %s",
            tokenTypeName(operator)
        );
    }

    if (rules.types_have_to_match && left_operand_type != right_operand_type) {
        error(
            parser,
            "Semantic",
            expression_start_token,
            previous(parser),
            "Expected %s's operands to be of the same type, but the types differ: %s and %s.",
            tokenTypeName(operator),
            basicValueTypeName(left_operand_type),
            basicValueTypeName(right_operand_type)
        );
    }

    ASSERT_PARSER(parser);
}


// ────────────────────
//  Operator to OpCode 
// ────────────────────

#define KEY(arity, token_type, value_type) ((token_type * 4 + value_type) * 3 + arity)

OpCode token_and_value_type_to_opcodes[][2] = {
    // Logic
    [KEY(2, TOKEN_OR,                 BASIC_VALUE_TYPE_BOOL)  ] = { OP_OR,                OP_EMPTY       },
    [KEY(2, TOKEN_AND,                BASIC_VALUE_TYPE_BOOL)  ] = { OP_AND,               OP_EMPTY       },
    
    // Comparison
    [KEY(2, TOKEN_EQUAL_EQUAL,        BASIC_VALUE_TYPE_BOOL)  ] = { OP_EQUALS_BOOL,       OP_EMPTY       },
    [KEY(2, TOKEN_EQUAL_EQUAL,        BASIC_VALUE_TYPE_INT)   ] = { OP_EQUALS_INT,        OP_EMPTY       },
    [KEY(2, TOKEN_EQUAL_EQUAL,        BASIC_VALUE_TYPE_FLOAT) ] = { OP_EQUALS_FLOAT,      OP_EMPTY       },
    [KEY(2, TOKEN_EQUAL_EQUAL,        BASIC_VALUE_TYPE_STRING)] = { OP_EQUALS_STRING,     OP_EMPTY       },
    
    [KEY(2, TOKEN_EXCLAMATION_EQUAL,  BASIC_VALUE_TYPE_BOOL)  ] = { OP_EQUALS_BOOL,       OP_NEGATE_BOOL },
    [KEY(2, TOKEN_EXCLAMATION_EQUAL,  BASIC_VALUE_TYPE_INT)   ] = { OP_EQUALS_INT,        OP_NEGATE_BOOL },
    [KEY(2, TOKEN_EXCLAMATION_EQUAL,  BASIC_VALUE_TYPE_FLOAT) ] = { OP_EQUALS_FLOAT,      OP_NEGATE_BOOL },
    [KEY(2, TOKEN_EXCLAMATION_EQUAL,  BASIC_VALUE_TYPE_STRING)] = { OP_EQUALS_STRING,     OP_NEGATE_BOOL },
    
    [KEY(2, TOKEN_GREATER_EQUAL,      BASIC_VALUE_TYPE_INT)   ] = { OP_LESS_INT,          OP_NEGATE_BOOL },
    [KEY(2, TOKEN_GREATER_EQUAL,      BASIC_VALUE_TYPE_FLOAT) ] = { OP_LESS_FLOAT,        OP_NEGATE_BOOL },
    [KEY(2, TOKEN_GREATER_EQUAL,      BASIC_VALUE_TYPE_STRING)] = { OP_LESS_STRING,       OP_NEGATE_BOOL },
    
    [KEY(2, TOKEN_LESS_EQUAL,         BASIC_VALUE_TYPE_INT)   ] = { OP_GREATER_INT,       OP_NEGATE_BOOL },
    [KEY(2, TOKEN_LESS_EQUAL,         BASIC_VALUE_TYPE_FLOAT) ] = { OP_GREATER_FLOAT,     OP_NEGATE_BOOL },
    [KEY(2, TOKEN_LESS_EQUAL,         BASIC_VALUE_TYPE_STRING)] = { OP_GREATER_STRING,    OP_NEGATE_BOOL },
    
    [KEY(2, TOKEN_GREATER,            BASIC_VALUE_TYPE_INT)   ] = { OP_GREATER_INT,       OP_EMPTY       },
    [KEY(2, TOKEN_GREATER,            BASIC_VALUE_TYPE_FLOAT) ] = { OP_GREATER_FLOAT,     OP_EMPTY       },
    [KEY(2, TOKEN_GREATER,            BASIC_VALUE_TYPE_STRING)] = { OP_GREATER_STRING,    OP_EMPTY       },
    
    [KEY(2, TOKEN_LESS,               BASIC_VALUE_TYPE_INT)   ] = { OP_LESS_INT,          OP_EMPTY       },
    [KEY(2, TOKEN_LESS,               BASIC_VALUE_TYPE_FLOAT) ] = { OP_LESS_FLOAT,        OP_EMPTY       },
    [KEY(2, TOKEN_LESS,               BASIC_VALUE_TYPE_STRING)] = { OP_LESS_STRING,       OP_EMPTY       },
    
    // Arithmetics
    [KEY(2, TOKEN_PLUS,               BASIC_VALUE_TYPE_INT)   ] = { OP_ADD_INT,           OP_EMPTY       },
    [KEY(2, TOKEN_PLUS,               BASIC_VALUE_TYPE_FLOAT) ] = { OP_ADD_FLOAT,         OP_EMPTY       },
    [KEY(2, TOKEN_PLUS,               BASIC_VALUE_TYPE_STRING)] = { OP_CONCATENATE,       OP_EMPTY       },
    
    [KEY(2, TOKEN_MINUS,              BASIC_VALUE_TYPE_INT)   ] = { OP_NEGATE_INT,        OP_ADD_INT     },
    [KEY(2, TOKEN_MINUS,              BASIC_VALUE_TYPE_FLOAT) ] = { OP_NEGATE_FLOAT,      OP_ADD_FLOAT   },
    
    [KEY(2, TOKEN_STAR,               BASIC_VALUE_TYPE_INT)   ] = { OP_MULTIPLY_INT,      OP_EMPTY       },
    [KEY(2, TOKEN_STAR,               BASIC_VALUE_TYPE_FLOAT) ] = { OP_MULTIPLY_FLOAT,    OP_EMPTY       },
    
    [KEY(2, TOKEN_SLASH,              BASIC_VALUE_TYPE_INT)   ] = { OP_DIVIDE_INT,        OP_EMPTY       },
    [KEY(2, TOKEN_SLASH,              BASIC_VALUE_TYPE_FLOAT) ] = { OP_DIVIDE_FLOAT,      OP_EMPTY       },
    
    [KEY(2, TOKEN_PERCENT,            BASIC_VALUE_TYPE_INT)   ] = { OP_MODULO_INT,        OP_EMPTY       },
    [KEY(2, TOKEN_PERCENT,            BASIC_VALUE_TYPE_FLOAT) ] = { OP_MODULO_FLOAT,      OP_EMPTY       },

    // Unary
    [KEY(1, TOKEN_EXCLAMATION,        BASIC_VALUE_TYPE_BOOL)  ] = { OP_NEGATE_BOOL,       OP_EMPTY       },
    [KEY(1, TOKEN_MINUS,              BASIC_VALUE_TYPE_INT)   ] = { OP_NEGATE_INT,        OP_EMPTY       },
    [KEY(1, TOKEN_MINUS,              BASIC_VALUE_TYPE_FLOAT) ] = { OP_NEGATE_FLOAT,      OP_EMPTY       },
};

static void emitOpCodesForTokenAndValueTypesCombination(
    Parser* parser,
    uint8_t arity,
    TokenType token_type,
    BasicValueType value_type
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

OpCode getOpGetFromStackForValueType(
    ValueType* value_type, 
    VariableKind kind
) {
    switch (value_type->basic_type) {
        case BASIC_VALUE_TYPE_BOOL:
            return kind == LOCAL_VARIABLE ? OP_GET_LOCAL_BYTE : OP_GET_GLOBAL_BYTE;
        case BASIC_VALUE_TYPE_INT:
            return kind == LOCAL_VARIABLE ? OP_GET_LOCAL_INT : OP_GET_GLOBAL_INT;
        case BASIC_VALUE_TYPE_FLOAT:
            return kind == LOCAL_VARIABLE ? OP_GET_LOCAL_FLOAT : OP_GET_GLOBAL_FLOAT;

        case BASIC_VALUE_TYPE_STRING:
        case BASIC_VALUE_TYPE_ARRAY:
        case BASIC_VALUE_TYPE_MAP:
        case BASIC_VALUE_TYPE_FUNCTION:
        case BASIC_VALUE_TYPE_REFERENCE_STRUCTURE:
        case BASIC_VALUE_TYPE_OBJECT:
            return kind == LOCAL_VARIABLE ? OP_GET_LOCAL_ADDRESS : OP_GET_GLOBAL_ADDRESS;

        case BASIC_VALUE_TYPE_VOID:
        case BASIC_VALUE_TYPE_PLAIN_STRUCTURE:
        default:
            assert(false);
    }
}

OpCode getOpSetOnStackForValueType(
    ValueType* value_type,
    VariableKind kind
) {
    switch (value_type->basic_type) {
        case BASIC_VALUE_TYPE_BOOL:
            return kind == LOCAL_VARIABLE ? OP_SET_LOCAL_BYTE : OP_SET_GLOBAL_BYTE;
        case BASIC_VALUE_TYPE_INT:
            return kind == LOCAL_VARIABLE ? OP_SET_LOCAL_INT : OP_SET_GLOBAL_INT;
        case BASIC_VALUE_TYPE_FLOAT:
            return kind == LOCAL_VARIABLE ? OP_SET_LOCAL_FLOAT : OP_SET_GLOBAL_FLOAT;

        case BASIC_VALUE_TYPE_STRING:
        case BASIC_VALUE_TYPE_ARRAY:
        case BASIC_VALUE_TYPE_MAP:
        case BASIC_VALUE_TYPE_FUNCTION:
        case BASIC_VALUE_TYPE_OBJECT:
            return kind == LOCAL_VARIABLE ? OP_SET_LOCAL_ADDRESS : OP_SET_GLOBAL_ADDRESS;

        case BASIC_VALUE_TYPE_VOID:
        case BASIC_VALUE_TYPE_PLAIN_STRUCTURE:
        case BASIC_VALUE_TYPE_REFERENCE_STRUCTURE:
        default:
            assert(false);
    }
}


#undef VALIDATE_PARSER
#undef ASSERT_PARSER

