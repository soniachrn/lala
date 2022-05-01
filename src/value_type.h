#ifndef lala_value_type_h
#define lala_value_type_h


#include <stdbool.h>
#include <stdlib.h>


// ┌───────┐
// │ Types │
// └───────┘

typedef enum {
    BASIC_VALUE_TYPE_INVALID,
    BASIC_VALUE_TYPE_BOOL,
    BASIC_VALUE_TYPE_INT,
    BASIC_VALUE_TYPE_FLOAT,
    BASIC_VALUE_TYPE_STRING,
    BASIC_VALUE_TYPE_ARRAY,
    BASIC_VALUE_TYPE_MAP,
    // VALUE_USER_DEFINED,
} BasicValueType;

struct ValueType;
typedef struct ValueType ValueType;

typedef struct {
    ValueType* element_type;
} ArrayValueType;

typedef struct {
    ValueType* key_type;
    ValueType* element_type;
} MapValueType;

struct ValueType {
    BasicValueType basic_type;
    union {
        ArrayValueType array;
        MapValueType   map;
    } as;
    char* name;
};


// ┌────────────────────────┐
// │ Constants declarations │
// └────────────────────────┘

extern ValueType VALUE_TYPE_INVALID;
extern ValueType VALUE_TYPE_BOOL;
extern ValueType VALUE_TYPE_INT;
extern ValueType VALUE_TYPE_FLOAT;
extern ValueType VALUE_TYPE_STRING;


// ┌───────────────────────┐
// │ Function declarations │
// └───────────────────────┘

ValueType* createArrayValueType(ValueType* element_type);
void       deleteValueType(ValueType* value_type);

const char* basicValueTypeName(BasicValueType basic_value_type);
const char* valueTypeName(ValueType* value_type);

size_t      valueTypeSize(ValueType* value_type);
bool        isReferenceValueType(ValueType* value_type);
bool        valueTypesEqual(ValueType* a, ValueType* b);


#endif

