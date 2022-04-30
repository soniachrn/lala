#include "value_type.h"


#include <assert.h>
#include <stdbool.h>


// ┌──────────────────────────┐
// │ Function implementations │
// └──────────────────────────┘

const char* valueTypeName(ValueType value_type) {
    switch (value_type) {
        case VALUE_INVALID: return "INVALID TYPE";
        case VALUE_BOOL:    return "bool";
        case VALUE_INT:     return "int";
        case VALUE_FLOAT:   return "float";
        case VALUE_STRING:  return "string";
    }
    
    return "INVALID_VALUE_TYPE";
}

size_t valueTypeSize(ValueType value_type) {
    switch (value_type) {
        case VALUE_BOOL:   return sizeof(uint8_t);
        case VALUE_INT:    return sizeof(uint32_t);
        case VALUE_FLOAT:  return sizeof(double);
        case VALUE_STRING: return sizeof(size_t);
        default: assert(false);
    }
}

