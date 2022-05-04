#ifndef lala_value_type_h
#define lala_value_type_h


#include <stdbool.h>
#include <stdlib.h>

#include "op_code.h"


// ┌───────┐
// │ Types │
// └───────┘

typedef enum {
    BASIC_VALUE_TYPE_INVALID,
    BASIC_VALUE_TYPE_VOID,
    BASIC_VALUE_TYPE_BOOL,
    BASIC_VALUE_TYPE_INT,
    BASIC_VALUE_TYPE_FLOAT,
    BASIC_VALUE_TYPE_STRING,
    BASIC_VALUE_TYPE_ARRAY,
    BASIC_VALUE_TYPE_MAP,
    BASIC_VALUE_TYPE_FUNCTION,
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

typedef struct {
    uint8_t arity;
    size_t parameters_size;
    ValueType** parameter_types;
    ValueType* return_type;
} FunctionValueType;

struct ValueType {
    BasicValueType basic_type;
    union {
        ArrayValueType    array;
        MapValueType      map;
        FunctionValueType function;
    } as;
    char* name;
};


// ┌────────────────────────┐
// │ Constants declarations │
// └────────────────────────┘

extern ValueType VALUE_TYPE_INVALID;
extern ValueType VALUE_TYPE_VOID;
extern ValueType VALUE_TYPE_BOOL;
extern ValueType VALUE_TYPE_INT;
extern ValueType VALUE_TYPE_FLOAT;
extern ValueType VALUE_TYPE_STRING;


// ┌───────────────────────┐
// │ Function declarations │
// └───────────────────────┘

ValueType* createArrayValueType(ValueType* element_type);
ValueType* createFunctionValueType();
void addParameterToFunctionValueType(
    FunctionValueType* function,
    ValueType* parameter
);
void deleteValueType(ValueType* value_type);

const char* basicValueTypeName(BasicValueType basic_value_type);
const char* valueTypeName(ValueType* value_type);

size_t valueTypeSize(ValueType* value_type);
bool   isReferenceValueType(ValueType* value_type);
bool   valueTypesEqual(ValueType* a, ValueType* b);

OpCode getOpPopForValueType(ValueType* value_type);
OpCode getOpReturnForValueType(ValueType* value_type);


#endif

