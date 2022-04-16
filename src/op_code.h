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
    OP_ADD_INT = 0x13,
    OP_ADD_FLOAT = 0x14,
    OP_MULTIPLY_INT = 0x15,
    OP_MULTIPLY_FLOAT = 0x16,
    OP_DIVIDE_INT = 0x17,
    OP_DIVIDE_FLOAT = 0x18,
    OP_MODULO_INT = 0x19,
    OP_MODULO_FLOAT = 0x1A,
    OP_NEGATE_INT = 0x1B,
    OP_NEGATE_FLOAT = 0x1C,

    // String
    OP_CONCATENATE = 0x1D,

    // Cast
    OP_CAST_FLOAT_TO_INT = 0x1E,
    OP_CAST_INT_TO_FLOAT = 0x1F,

    // Local variables
    // The duplication may be resolved by adding another
    // parameter for OP_GET / OP_SET commands, but 
    // there's no need in it at the moment and its' 
    // faster the way it is now anyway.
    OP_GET_BYTE_FROM_STACK = 0x20,
    OP_GET_INT_FROM_STACK = 0x21,
    OP_GET_FLOAT_FROM_STACK = 0x22,
    OP_SET_BYTE_ON_STACK = 0x23,
    OP_SET_INT_ON_STACK = 0x24,
    OP_SET_FLOAT_ON_STACK = 0x25,

    // Print
    OP_PRINT_BOOL = 0x26,
    OP_PRINT_INT = 0x27,
    OP_PRINT_FLOAT = 0x28,

    // Empty
    OP_EMPTY,
} OpCode;


#endif

