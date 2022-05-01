#include "op_code.h"


const char* opCodeName(OpCode op_code) {
    switch (op_code) {
        // Push
        case OP_PUSH_TRUE:               return "push true";
        case OP_PUSH_FALSE:              return "push false";
        case OP_PUSH_INT:                return "push int";
        case OP_PUSH_FLOAT:              return "push float";
        case OP_LOAD_CONSTANT:           return "load constant";
        case OP_DEFINE_ON_HEAP:          return "define on heap";

        // Logical
        case OP_OR:                      return "or";
        case OP_AND:                     return "and";
        case OP_NEGATE_BOOL:             return "negate bool";

        // Comparison
        case OP_EQUALS_BOOL:             return "equals bool";
        case OP_EQUALS_INT:              return "equals int";
        case OP_EQUALS_FLOAT:            return "equals float";
        case OP_EQUALS_STRING:           return "equals string";
        case OP_LESS_INT:                return "less int";
        case OP_LESS_FLOAT:              return "less float";
        case OP_LESS_STRING:             return "less string";
        case OP_GREATER_INT:             return "greater int";
        case OP_GREATER_FLOAT:           return "greater float";
        case OP_GREATER_STRING:          return "greater string";

        // Math
        case OP_ADD_INT:                 return "add int";
        case OP_ADD_FLOAT:               return "add float";
        case OP_MULTIPLY_INT:            return "multiply int";
        case OP_MULTIPLY_FLOAT:          return "multiply float";
        case OP_DIVIDE_INT:              return "divide int";
        case OP_DIVIDE_FLOAT:            return "divide float";
        case OP_MODULO_INT:              return "modulo int";
        case OP_MODULO_FLOAT:            return "modulo float";
        case OP_NEGATE_INT:              return "negate int";
        case OP_NEGATE_FLOAT:            return "negate float";

        // String
        case OP_CONCATENATE:             return "concatenate";

        // Cast
        case OP_CAST_FLOAT_TO_INT:       return "cast float to int";
        case OP_CAST_INT_TO_FLOAT:       return "cast int to float";
        case OP_CAST_BOOL_TO_STRING:     return "cast bool to string";
        case OP_CAST_INT_TO_STRING:      return "cast int to string";
        case OP_CAST_FLOAT_TO_STRING:    return "cast float to string";

        // Local variables
        case OP_GET_BYTE_FROM_STACK:     return "get byte from stack";
        case OP_GET_INT_FROM_STACK:      return "get int from stack";
        case OP_GET_FLOAT_FROM_STACK:    return "get float from stack";
        case OP_GET_ADDRESS_FROM_STACK:  return "get address from stack";

        case OP_SET_BYTE_ON_STACK:       return "set byte on stack";
        case OP_SET_INT_ON_STACK:        return "set int on stack";
        case OP_SET_FLOAT_ON_STACK:      return "set float on stack";
        case OP_SET_ADDRESS_ON_STACK:    return "set address on stack";

        // Print
        case OP_PRINT_BOOL:              return "print bool";
        case OP_PRINT_INT:               return "print int";
        case OP_PRINT_FLOAT:             return "print float";
        case OP_PRINT_STRING:            return "print string";

        // Jump
        case OP_JUMP:                    return "jump";
        case OP_JUMP_IF_TRUE:            return "jump if true";
        case OP_JUMP_IF_FALSE:           return "jump if false";

        // Empty
        case OP_EMPTY:                   return "empty";

        // Array
        case OP_SUBSCRIPT_BYTE:          return "subscript byte";
        case OP_SUBSCRIPT_INT:           return "subscript int";
        case OP_SUBSCRIPT_FLOAT:         return "subscript float";
        case OP_SUBSCRIPT_ADDRESS:       return "subscript address";

        default:                         return "INVALID";
    }
}

