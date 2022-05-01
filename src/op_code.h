#ifndef lala_op_code_h
#define lala_op_code_h


// ┌───────┐
// │ Types │
// └───────┘

typedef enum {
    // Empty
    OP_EMPTY,

    // Push
    OP_PUSH_TRUE,
    OP_PUSH_FALSE,
    OP_PUSH_INT,
    OP_PUSH_FLOAT,
    OP_LOAD_CONSTANT,
    OP_DEFINE_ON_HEAP,

    // Logical
    OP_OR,
    OP_AND,
    OP_NEGATE_BOOL,

    // Comparison
    OP_EQUALS_BOOL,
    OP_EQUALS_INT,
    OP_EQUALS_FLOAT,
    OP_EQUALS_STRING,
    OP_LESS_INT,
    OP_LESS_FLOAT,
    OP_LESS_STRING,
    OP_GREATER_INT,
    OP_GREATER_FLOAT,
    OP_GREATER_STRING,

    // Math
    OP_ADD_INT,
    OP_ADD_FLOAT,
    OP_MULTIPLY_INT,
    OP_MULTIPLY_FLOAT,
    OP_DIVIDE_INT,
    OP_DIVIDE_FLOAT,
    OP_MODULO_INT,
    OP_MODULO_FLOAT,
    OP_NEGATE_INT,
    OP_NEGATE_FLOAT,

    // String
    OP_CONCATENATE,

    // Cast
    OP_CAST_FLOAT_TO_INT,
    OP_CAST_INT_TO_FLOAT,
    OP_CAST_BOOL_TO_STRING,
    OP_CAST_INT_TO_STRING,
    OP_CAST_FLOAT_TO_STRING,

    // Local variables
    OP_GET_BYTE_FROM_STACK,
    OP_GET_INT_FROM_STACK,
    OP_GET_FLOAT_FROM_STACK,
    OP_GET_ADDRESS_FROM_STACK,

    OP_SET_BYTE_ON_STACK,
    OP_SET_INT_ON_STACK,
    OP_SET_FLOAT_ON_STACK,
    OP_SET_ADDRESS_ON_STACK,

    // Print
    OP_PRINT_BOOL,
    OP_PRINT_INT,
    OP_PRINT_FLOAT,
    OP_PRINT_STRING,

    // Jump
    OP_JUMP,
    OP_JUMP_IF_TRUE,
    OP_JUMP_IF_FALSE,

    // Array
    OP_SUBSCRIPT_BYTE,
    OP_SUBSCRIPT_INT,
    OP_SUBSCRIPT_FLOAT,
    OP_SUBSCRIPT_ADDRESS,
} OpCode;


const char* opCodeName(OpCode op_code);


#endif

