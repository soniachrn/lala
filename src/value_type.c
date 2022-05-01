#include "value_type.h"


#include <assert.h>
#include <stdio.h>


// ┌───────────────────────┐
// │ Constants definitions │
// └───────────────────────┘

ValueType VALUE_TYPE_INVALID = { BASIC_VALUE_TYPE_INVALID, {{NULL}}, NULL };
ValueType VALUE_TYPE_BOOL    = { BASIC_VALUE_TYPE_BOOL,    {{NULL}}, NULL };
ValueType VALUE_TYPE_INT     = { BASIC_VALUE_TYPE_INT,     {{NULL}}, NULL };
ValueType VALUE_TYPE_FLOAT   = { BASIC_VALUE_TYPE_FLOAT,   {{NULL}}, NULL };
ValueType VALUE_TYPE_STRING  = { BASIC_VALUE_TYPE_STRING,  {{NULL}}, NULL };

// ┌──────────────────────────┐
// │ Function implementations │
// └──────────────────────────┘

ValueType* createArrayValueType(ValueType* element_type) {
    ValueType* type = calloc(1, sizeof(ValueType));
    type->basic_type = BASIC_VALUE_TYPE_ARRAY;
    type->as.array.element_type = element_type;
    type->name = NULL;
    return type;
}

void deleteValueType(ValueType* value_type) {
    switch (value_type->basic_type) {
        case BASIC_VALUE_TYPE_INVALID:
        case BASIC_VALUE_TYPE_BOOL:
        case BASIC_VALUE_TYPE_INT:
        case BASIC_VALUE_TYPE_FLOAT:
        case BASIC_VALUE_TYPE_STRING:
            assert(false);
        default:
            break;
    }

    if (value_type->name)
        free(value_type->name);
    free(value_type);
}

const char* basicValueTypeName(BasicValueType basic_value_type) {
    switch (basic_value_type) {
        case BASIC_VALUE_TYPE_INVALID: return "INVALID TYPE";
        case BASIC_VALUE_TYPE_BOOL:    return "bool";
        case BASIC_VALUE_TYPE_INT:     return "int";
        case BASIC_VALUE_TYPE_FLOAT:   return "float";
        case BASIC_VALUE_TYPE_STRING:  return "string";
        case BASIC_VALUE_TYPE_ARRAY:   return "array";
        case BASIC_VALUE_TYPE_MAP:     return "map";
        default:                       return "INVALID TYPE";
    }
}

const char* valueTypeName(ValueType* value_type) {
    switch (value_type->basic_type) {
        case BASIC_VALUE_TYPE_INVALID:
        case BASIC_VALUE_TYPE_BOOL:
        case BASIC_VALUE_TYPE_INT:
        case BASIC_VALUE_TYPE_FLOAT:
        case BASIC_VALUE_TYPE_STRING:
            return basicValueTypeName(value_type->basic_type);

        case BASIC_VALUE_TYPE_ARRAY:
            if (!value_type->name) {
                value_type->name = calloc(128, sizeof(char));
                int length = snprintf(
                    value_type->name, 128, "[%s]",
                    valueTypeName(value_type->as.array.element_type)
                );
                value_type->name = realloc(value_type->name, (size_t)length * sizeof(char) + 1);
            }
            return value_type->name;

        case BASIC_VALUE_TYPE_MAP:
            if (!value_type->name) {
                value_type->name = calloc(256, sizeof(char));
                int length = snprintf(
                    value_type->name, 256, "{%s:%s}",
                    valueTypeName(value_type->as.map.key_type),
                    valueTypeName(value_type->as.map.element_type)
                );
                value_type->name = realloc(value_type->name, (size_t)length * sizeof(char) + 1);
            }
            return value_type->name;

        default:
            return "INVALID TYPE";
    }
}

size_t valueTypeSize(ValueType* value_type) {
    switch (value_type->basic_type) {
        case BASIC_VALUE_TYPE_BOOL:   return sizeof(uint8_t);
        case BASIC_VALUE_TYPE_INT:    return sizeof(uint32_t);
        case BASIC_VALUE_TYPE_FLOAT:  return sizeof(double);

        case BASIC_VALUE_TYPE_STRING:
        case BASIC_VALUE_TYPE_ARRAY:
        case BASIC_VALUE_TYPE_MAP:
            return sizeof(size_t);

        default:
            assert(false);
    }
}

bool isReferenceValueType(ValueType* value_type) {
    switch (value_type->basic_type) {
        case BASIC_VALUE_TYPE_BOOL:
        case BASIC_VALUE_TYPE_INT:
        case BASIC_VALUE_TYPE_FLOAT:
            return false;

        case BASIC_VALUE_TYPE_STRING:
        case BASIC_VALUE_TYPE_ARRAY:
        case BASIC_VALUE_TYPE_MAP:
            return true;

        default:
            assert(false);
    }
}

bool valueTypesEqual(ValueType* a, ValueType* b) {
    assert(a);
    assert(b);

    if (a->basic_type != b->basic_type) {
        return false;
    }

    switch (a->basic_type) {
        case BASIC_VALUE_TYPE_BOOL:
        case BASIC_VALUE_TYPE_INT:
        case BASIC_VALUE_TYPE_FLOAT:
        case BASIC_VALUE_TYPE_STRING:
            return true;

        case BASIC_VALUE_TYPE_ARRAY:
            return valueTypesEqual(a->as.array.element_type, b->as.array.element_type);

        case BASIC_VALUE_TYPE_MAP:
            return (
                valueTypesEqual(a->as.map.key_type,     b->as.map.key_type) &&
                valueTypesEqual(a->as.map.element_type, b->as.map.element_type)
            );

        case BASIC_VALUE_TYPE_INVALID:
            assert(false);
        default:
            assert(false);
    }
}

