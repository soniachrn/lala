#ifndef lala_op_code_h
#define lala_op_code_h


// ┌───────┐
// │ Types │
// └───────┘

typedef enum {
    // Push
    OP_PUSH_TRUE  = 0x00,
    OP_PUSH_FALSE = 0x01,
    OP_PUSH_INT   = 0x02,
    OP_PUSH_FLOAT = 0x03,
    OP_ACCESS_CONSTANT_TABLE = 0x04,
    OP_ACCESS_SYMBOL_TABLE   = 0x05,

    // Logical
    OP_OR = 0x06,
    OP_AND = 0x07,
    OP_NEGATE_BOOL = 0x08,

    // Comparison
    OP_EQUALS_BOOL = 0x09,
    OP_EQUALS_INT = 0x0A,
    OP_EQUALS_FLOAT = 0x0B,
    OP_EQUALS_STRING = 0x0C,
    OP_LESS_INT = 0x0D,
    OP_LESS_FLOAT = 0x0E,
    OP_LESS_STRING = 0x0F,
    OP_GREATER_INT = 0x10,
    OP_GREATER_FLOAT = 0x11,
    OP_GREATER_STRING = 0x12,

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

    // Local variables
    // The duplication may be resolved by adding another
    // parameter for OP_GET / OP_SET commands, but 
    // there's no need in it at the moment and its' 
    // faster the way it is now anyway.
    OP_GET_BYTE_FROM_STACK,
    OP_GET_INT_FROM_STACK,
    OP_GET_FLOAT_FROM_STACK,
    OP_SET_BYTE_ON_STACK,
    OP_SET_INT_ON_STACK,
    OP_SET_FLOAT_ON_STACK,

    // Print
    OP_PRINT_BOOL,
    OP_PRINT_INT,
    OP_PRINT_FLOAT,

    // Empty
    OP_EMPTY,
} OpCode;


#endif

