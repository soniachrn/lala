#ifndef lala_value_type_h
#define lala_value_type_h


#include <stdlib.h>


// ┌───────┐
// │ Types │
// └───────┘

typedef enum {
    VALUE_BOOL,
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_STRING,
    // VALUE_ARRAY,
    // VALUE_MAP,
    // VALUE_USER_DEFINED,
} ValueType;


// ┌───────────────────────┐
// │ Function declarations │
// └───────────────────────┘

const char* valueTypeName(ValueType value_type);
size_t      valueTypeSize(ValueType value_type);


#endif

