#include "value_type.h"


#include <assert.h>
#include <stdbool.h>


// ┌──────────────────────────┐
// │ Function implementations │
// └──────────────────────────┘

const char* valueTypeName(ValueType value_type) {
    switch (value_type) {
        case VALUE_INVALID: return "INVALID TYPE";
        case VALUE_BOOL:    return "BOOL";
        case VALUE_INT:     return "INT";
        case VALUE_FLOAT:   return "FLOAT";
        case VALUE_STRING:  return "STRING";
    }
    
    return "INVALID_VALUE_TYPE";
}

size_t valueTypeSize(ValueType value_type) {
    switch (value_type) {
        case VALUE_BOOL:   return sizeof(uint8_t);
        case VALUE_INT:    return sizeof(uint32_t);
        case VALUE_FLOAT:  return sizeof(double);
        default: assert(false);
    }
}

