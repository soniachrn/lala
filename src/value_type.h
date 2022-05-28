#ifndef lala_value_type_h
#define lala_value_type_h


#include <stdbool.h>
#include <stdlib.h>

#include "hashmap.h"
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

    // Plain structure doesn't have reference fields 
    // and doesn't have a runtime representation.
    BASIC_VALUE_TYPE_PLAIN_STRUCTURE,
    // Reference structure has at least one reference field
    // and has a runtime representation.
    BASIC_VALUE_TYPE_REFERENCE_STRUCTURE,

    BASIC_VALUE_TYPE_OBJECT,
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

typedef struct {
    ValueType* type;
    size_t offset;
} Field;

typedef struct {
    HashMap fields_map;
    Field* fields_properties;
    size_t size;
    ValueType* instance_type;
} StructureValueType;

typedef struct {
    const ValueType* structure_type;
} ObjectValueType;

struct ValueType {
    BasicValueType basic_type;
    union {
        ArrayValueType     array;
        MapValueType       map;
        FunctionValueType  function;
        StructureValueType structure;
        ObjectValueType    object;
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
ValueType* createStructureValueType(const char* name, uint8_t name_length);
bool addFieldToStructureValueType(
    ValueType* value_type,
    const char* field_name,
    uint8_t field_name_length,
    ValueType* field_type
);
ValueType* createObjectValueType(const ValueType* structure_type);
void deleteValueType(ValueType* value_type);

const char* basicValueTypeName(BasicValueType basic_value_type);
const char* valueTypeName(ValueType* value_type);

// Value type size on stack.
size_t valueTypeSize(ValueType* value_type);
bool   isReferenceValueType(ValueType* value_type);
bool   isStructureValueType(ValueType* value_type);
bool   valueTypesEqual(ValueType* a, ValueType* b);

OpCode getOpPopForValueType         (ValueType* value_type);
OpCode getOpReturnForValueType      (ValueType* value_type);
OpCode getOpGetFromHeapForValueType (ValueType* value_type);
OpCode getOpSetOnHeapForValueType   (ValueType* value_type);
OpCode getOpSubscriptGetForValueType(ValueType* value_type);
OpCode getOpSubscriptSetForValueType(ValueType* value_type);


#endif

