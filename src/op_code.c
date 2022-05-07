#include "op_code.h"


const char* opCodeName(OpCode op_code) {
    switch (op_code) {
        // Empty
        case OP_EMPTY:                   return "empty";

        // Stack
        case OP_PUSH_TRUE:               return "push true";
        case OP_PUSH_FALSE:              return "push false";
        case OP_PUSH_BYTE:               return "push byte";
        case OP_PUSH_INT:                return "push int";
        case OP_PUSH_FLOAT:              return "push float";
        case OP_PUSH_ADDRESS:            return "push address";
        case OP_POP_BYTE:                return "pop byte";
        case OP_POP_INT:                 return "pop int";
        case OP_POP_FLOAT:               return "pop float";
        case OP_POP_ADDRESS:             return "pop address";
        case OP_POP_BYTES:               return "pop bytes";

        // Heap
        case OP_LOAD_CONSTANT:           return "load constant";
        case OP_DEFINE_ON_HEAP:          return "define on heap";
        
        case OP_GET_BYTE_FROM_HEAP:      return "get byte from heap";
        case OP_GET_INT_FROM_HEAP:       return "get int from heap";
        case OP_GET_FLOAT_FROM_HEAP:     return "get float from heap";
        case OP_GET_ADDRESS_FROM_HEAP:   return "get address from heap";

        case OP_SET_BYTE_ON_HEAP:        return "set byte on heap";
        case OP_SET_INT_ON_HEAP:         return "set int on heap";
        case OP_SET_FLOAT_ON_HEAP:       return "set float on heap";
        case OP_SET_ADDRESS_ON_HEAP:     return "set address on heap";

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

        // Variables
        case OP_GET_LOCAL_BYTE:           return "get local byte";
        case OP_GET_LOCAL_INT:            return "get local int";
        case OP_GET_LOCAL_FLOAT:          return "get local float";
        case OP_GET_LOCAL_ADDRESS:        return "get local address";

        case OP_SET_LOCAL_BYTE:           return "set local byte";
        case OP_SET_LOCAL_INT:            return "set local int";
        case OP_SET_LOCAL_FLOAT:          return "set local float";
        case OP_SET_LOCAL_ADDRESS:        return "set local address";

        case OP_GET_GLOBAL_BYTE:           return "get global byte";
        case OP_GET_GLOBAL_INT:            return "get global int";
        case OP_GET_GLOBAL_FLOAT:          return "get global float";
        case OP_GET_GLOBAL_ADDRESS:        return "get global address";

        case OP_SET_GLOBAL_BYTE:           return "set global byte";
        case OP_SET_GLOBAL_INT:            return "set global int";
        case OP_SET_GLOBAL_FLOAT:          return "set global float";
        case OP_SET_GLOBAL_ADDRESS:        return "set global address";

        // Print
        case OP_PRINT_BOOL:              return "print bool";
        case OP_PRINT_INT:               return "print int";
        case OP_PRINT_FLOAT:             return "print float";
        case OP_PRINT_STRING:            return "print string";

        // Jump
        case OP_JUMP:                    return "jump";
        case OP_JUMP_IF_TRUE:            return "jump if true";
        case OP_JUMP_IF_FALSE:           return "jump if false";

        // Functions
        case OP_CALL:                    return "call";
        case OP_RETURN_VOID:             return "return void";
        case OP_RETURN_BYTE:             return "return byte";
        case OP_RETURN_INT:              return "return int";
        case OP_RETURN_FLOAT:            return "return float";
        case OP_RETURN_ADDRESS:          return "return address";

        // Array
        case OP_SUBSCRIPT_BYTE:          return "subscript byte";
        case OP_SUBSCRIPT_INT:           return "subscript int";
        case OP_SUBSCRIPT_FLOAT:         return "subscript float";
        case OP_SUBSCRIPT_ADDRESS:       return "subscript address";

        default:                         return "INVALID";
    }
}

