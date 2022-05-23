#include "value_type.h"


#include <assert.h>
#include <stdio.h>


// ┌───────────────────────┐
// │ Constants definitions │
// └───────────────────────┘

ValueType VALUE_TYPE_INVALID = { BASIC_VALUE_TYPE_INVALID, {{NULL}}, NULL };
ValueType VALUE_TYPE_VOID    = { BASIC_VALUE_TYPE_VOID,    {{NULL}}, NULL };
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

ValueType* createFunctionValueType() {
    ValueType* type = calloc(1, sizeof(ValueType));
    type->basic_type = BASIC_VALUE_TYPE_FUNCTION;
    type->as.function.arity = 0;
    type->as.function.parameters_size = 0;
    type->as.function.parameter_types = NULL;
    type->as.function.return_type = NULL;
    type->name = NULL;
    return type;
}

void addParameterToFunctionValueType(
    FunctionValueType* function,
    ValueType* parameter
) {
    assert(function);
    assert(function->arity < 255);

    function->arity += 1;
    function->parameters_size += valueTypeSize(parameter);
    function->parameter_types = realloc(
        function->parameter_types,
        function->arity * sizeof(ValueType*)
    );
    function->parameter_types[function->arity - 1] = parameter;
}

ValueType* createStructureValueType(const char* name, uint8_t name_length) {
    assert(name);

    ValueType* type = calloc(1, sizeof(ValueType));
    type->basic_type = BASIC_VALUE_TYPE_PLAIN_STRUCTURE;

    initHashMap(&type->as.structure.fields_map);
    type->as.structure.fields_properties = NULL;
    type->as.structure.size = 0;

    type->name = malloc(name_length + 1);
    memcpy(type->name, name, name_length);
    type->name[name_length] = '\0';

    type->as.structure.instance_type = createObjectValueType(type);

    return type;
}

bool addFieldToStructureValueType(
    ValueType* value_type,
    const char* field_name,
    uint8_t field_name_length,
    ValueType* field_type
) {
    assert(value_type);
    assert(
        value_type->basic_type == BASIC_VALUE_TYPE_PLAIN_STRUCTURE || 
        value_type->basic_type == BASIC_VALUE_TYPE_REFERENCE_STRUCTURE
    );
    assert(field_type);

    StructureValueType* structure = &value_type->as.structure;
    assert(structure->fields_map.count < 255);

    size_t field_index = structure->fields_map.count;

    if (!storeInHashMap(
        &structure->fields_map,
        field_name,
        field_name_length,
        field_index
    )) {
        return false;
    }

    structure->fields_properties = realloc(
        structure->fields_properties,
        (field_index + 1) * sizeof(Field)
    );
    structure->fields_properties[field_index].type   = field_type;
    structure->fields_properties[field_index].offset = structure->size;

    if (isReferenceValueType(field_type)) {
        value_type->basic_type = BASIC_VALUE_TYPE_REFERENCE_STRUCTURE;
    }

    structure->size += valueTypeSize(field_type);
    return true;
}

ValueType* createObjectValueType(const ValueType* structure_type) {
    assert(structure_type);

    ValueType* type = calloc(1, sizeof(ValueType));
    type->basic_type = BASIC_VALUE_TYPE_OBJECT;
    type->as.object.structure_type = structure_type;
    type->name = structure_type->name;
    return type;
}

void deleteValueType(ValueType* value_type) {
    assert(value_type);

    switch (value_type->basic_type) {
        case BASIC_VALUE_TYPE_INVALID:
        case BASIC_VALUE_TYPE_VOID:
        case BASIC_VALUE_TYPE_BOOL:
        case BASIC_VALUE_TYPE_INT:
        case BASIC_VALUE_TYPE_FLOAT:
        case BASIC_VALUE_TYPE_STRING:
        case BASIC_VALUE_TYPE_OBJECT:
            assert(false);

        case BASIC_VALUE_TYPE_FUNCTION:
            if (value_type->as.function.parameter_types) {
                free(value_type->as.function.parameter_types);
            }
            break;

        case BASIC_VALUE_TYPE_PLAIN_STRUCTURE:
        case BASIC_VALUE_TYPE_REFERENCE_STRUCTURE:
            freeHashMap(&value_type->as.structure.fields_map);
            if (value_type->as.structure.fields_properties) {
                free(value_type->as.structure.fields_properties);
            }
            free(value_type->as.structure.instance_type);
            break;

        default:
            break;
    }

    if (value_type->name)
        free(value_type->name);
    free(value_type);
}

const char* basicValueTypeName(BasicValueType basic_value_type) {
    switch (basic_value_type) {
        case BASIC_VALUE_TYPE_INVALID:   return "INVALID TYPE";
        case BASIC_VALUE_TYPE_VOID:      return "void";
        case BASIC_VALUE_TYPE_BOOL:      return "bool";
        case BASIC_VALUE_TYPE_INT:       return "int";
        case BASIC_VALUE_TYPE_FLOAT:     return "float";
        case BASIC_VALUE_TYPE_STRING:    return "string";
        case BASIC_VALUE_TYPE_ARRAY:     return "array";
        case BASIC_VALUE_TYPE_MAP:       return "map";
        case BASIC_VALUE_TYPE_FUNCTION:  return "function";

        case BASIC_VALUE_TYPE_PLAIN_STRUCTURE:
        case BASIC_VALUE_TYPE_REFERENCE_STRUCTURE:
            return "structure";

        case BASIC_VALUE_TYPE_OBJECT:    return "object";
        default:                         return "INVALID TYPE";
    }
}

const char* valueTypeName(ValueType* value_type) {
    switch (value_type->basic_type) {
        case BASIC_VALUE_TYPE_INVALID:
        case BASIC_VALUE_TYPE_VOID:
        case BASIC_VALUE_TYPE_BOOL:
        case BASIC_VALUE_TYPE_INT:
        case BASIC_VALUE_TYPE_FLOAT:
        case BASIC_VALUE_TYPE_STRING:
            return basicValueTypeName(value_type->basic_type);

#define INIT_VALUE_TYPE_NAME_IF_NEEDED(...)                   \
    if (!value_type->name) {                                  \
        size_t buffer_size = 128;                             \
        value_type->name = calloc(buffer_size, sizeof(char)); \
        size_t length = (size_t)snprintf(                     \
            value_type->name,                                 \
            buffer_size,                                      \
            __VA_ARGS__                                       \
        ) + 1;                                                \
        value_type->name = realloc(value_type->name, length); \
        if (length > buffer_size) {                           \
            snprintf(value_type->name, length, __VA_ARGS__);  \
        }                                                     \
    }

        case BASIC_VALUE_TYPE_ARRAY:
            INIT_VALUE_TYPE_NAME_IF_NEEDED(
                "[%s]", 
                valueTypeName(value_type->as.array.element_type)
            );
            return value_type->name;

        case BASIC_VALUE_TYPE_MAP:
            INIT_VALUE_TYPE_NAME_IF_NEEDED(
                "{%s:%s}",
                valueTypeName(value_type->as.map.key_type),
                valueTypeName(value_type->as.map.element_type)
            );
            return value_type->name;

#undef INIT_VALUE_TYPE_NAME

        case BASIC_VALUE_TYPE_FUNCTION:
// open_memstream isn't supported on windows.
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            return basicValueTypeName(value_type->basic_type);
#else
            if (!value_type->name) {
                FunctionValueType function = value_type->as.function;

                value_type->name = NULL;
                size_t length = 0;
                FILE* stream = open_memstream(&value_type->name, &length);

                fprintf(stream, "function (");
                for (uint8_t i = 0; i < function.arity; ++i) {
                    fprintf(
                        stream,
                        "%s",
                        valueTypeName(function.parameter_types[i])
                    );
                    if (i < function.arity - 1) {
                        fprintf(stream, ", ");
                    }
                }
                fprintf(
                    stream, 
                    "): %s", 
                    valueTypeName(function.return_type)
                );

                fclose(stream);
            }
            return value_type->name;
#endif

        case BASIC_VALUE_TYPE_PLAIN_STRUCTURE:
        case BASIC_VALUE_TYPE_REFERENCE_STRUCTURE:
        case BASIC_VALUE_TYPE_OBJECT:
            assert(value_type->name);
            return value_type->name;

        default:
            return "INVALID TYPE";
    }
}

size_t valueTypeSize(ValueType* value_type) {
    switch (value_type->basic_type) {
        case BASIC_VALUE_TYPE_PLAIN_STRUCTURE:
            return 0;

        case BASIC_VALUE_TYPE_BOOL:   return sizeof(uint8_t);
        case BASIC_VALUE_TYPE_INT:    return sizeof(uint32_t);
        case BASIC_VALUE_TYPE_FLOAT:  return sizeof(double);

        case BASIC_VALUE_TYPE_STRING:
        case BASIC_VALUE_TYPE_ARRAY:
        case BASIC_VALUE_TYPE_MAP:
        case BASIC_VALUE_TYPE_FUNCTION:
        case BASIC_VALUE_TYPE_REFERENCE_STRUCTURE:
        case BASIC_VALUE_TYPE_OBJECT:
            return sizeof(size_t);

        case BASIC_VALUE_TYPE_VOID:
        default:
            assert(false);
    }
}

bool isReferenceValueType(ValueType* value_type) {
    switch (value_type->basic_type) {
        case BASIC_VALUE_TYPE_BOOL:
        case BASIC_VALUE_TYPE_INT:
        case BASIC_VALUE_TYPE_FLOAT:
        case BASIC_VALUE_TYPE_FUNCTION:
            return false;

        case BASIC_VALUE_TYPE_STRING:
        case BASIC_VALUE_TYPE_ARRAY:
        case BASIC_VALUE_TYPE_MAP:
        case BASIC_VALUE_TYPE_OBJECT:
            return true;

        case BASIC_VALUE_TYPE_VOID:
        case BASIC_VALUE_TYPE_PLAIN_STRUCTURE:
        case BASIC_VALUE_TYPE_REFERENCE_STRUCTURE:
        default:
            assert(false);
    }
}

bool isStructureValueType(ValueType* value_type) {
    switch (value_type->basic_type) {
        case BASIC_VALUE_TYPE_PLAIN_STRUCTURE:
        case BASIC_VALUE_TYPE_REFERENCE_STRUCTURE:
            return true;
        default:
            return false;
    }
}

bool valueTypesEqual(ValueType* a, ValueType* b) {
    assert(a);
    assert(b);

    if (a->basic_type != b->basic_type) {
        return false;
    }

    switch (a->basic_type) {
        case BASIC_VALUE_TYPE_VOID:
        case BASIC_VALUE_TYPE_BOOL:
        case BASIC_VALUE_TYPE_INT:
        case BASIC_VALUE_TYPE_FLOAT:
        case BASIC_VALUE_TYPE_STRING:
            return true;

        case BASIC_VALUE_TYPE_ARRAY:
            return valueTypesEqual(
                a->as.array.element_type,
                b->as.array.element_type
            );

        case BASIC_VALUE_TYPE_MAP:
            return (
                valueTypesEqual(
                    a->as.map.key_type,
                    b->as.map.key_type
                ) &&
                valueTypesEqual(
                    a->as.map.element_type,
                    b->as.map.element_type
                )
            );

        case BASIC_VALUE_TYPE_FUNCTION: {
            bool equal = (
                a->as.function.arity == b->as.function.arity &&
                valueTypesEqual(
                    a->as.function.return_type,
                    b->as.function.return_type
                )
            );
            for (uint8_t i = 0; equal && i < a->as.function.arity; ++i) {
                equal &= valueTypesEqual(
                    a->as.function.parameter_types[i],
                    b->as.function.parameter_types[i]
                );
            }
            return equal;
        }

        case BASIC_VALUE_TYPE_PLAIN_STRUCTURE:
        case BASIC_VALUE_TYPE_REFERENCE_STRUCTURE:
        case BASIC_VALUE_TYPE_OBJECT:
            return a == b;

        case BASIC_VALUE_TYPE_INVALID:
            assert(false);
        default:
            assert(false);
    }
}

OpCode getOpPopForValueType(ValueType* value_type) {
    switch (value_type->basic_type) {
        case BASIC_VALUE_TYPE_BOOL:  return OP_POP_BYTE;
        case BASIC_VALUE_TYPE_INT:   return OP_POP_INT;
        case BASIC_VALUE_TYPE_FLOAT: return OP_POP_FLOAT;

        case BASIC_VALUE_TYPE_STRING:
        case BASIC_VALUE_TYPE_ARRAY:
        case BASIC_VALUE_TYPE_MAP:
        case BASIC_VALUE_TYPE_FUNCTION:
        case BASIC_VALUE_TYPE_REFERENCE_STRUCTURE:
        case BASIC_VALUE_TYPE_OBJECT:
            return OP_POP_ADDRESS;

        case BASIC_VALUE_TYPE_VOID:
        case BASIC_VALUE_TYPE_PLAIN_STRUCTURE:
            return OP_EMPTY;

        default:
            assert(false);
    }
}

OpCode getOpReturnForValueType(ValueType* value_type) {
    switch (value_type->basic_type) {
        case BASIC_VALUE_TYPE_VOID:  return OP_RETURN_VOID;
        case BASIC_VALUE_TYPE_BOOL:  return OP_RETURN_BYTE;
        case BASIC_VALUE_TYPE_INT:   return OP_RETURN_INT;
        case BASIC_VALUE_TYPE_FLOAT: return OP_RETURN_FLOAT;

        case BASIC_VALUE_TYPE_STRING:
        case BASIC_VALUE_TYPE_ARRAY:
        case BASIC_VALUE_TYPE_MAP:
        case BASIC_VALUE_TYPE_FUNCTION:
        case BASIC_VALUE_TYPE_OBJECT:
            return OP_RETURN_ADDRESS;

        case BASIC_VALUE_TYPE_PLAIN_STRUCTURE:
        case BASIC_VALUE_TYPE_REFERENCE_STRUCTURE:
        default:
            assert(false);
    }
}

OpCode getOpGetFromHeapForValueType(ValueType* value_type) {
    switch (value_type->basic_type) {
        case BASIC_VALUE_TYPE_BOOL:  return OP_GET_BYTE_FROM_HEAP;
        case BASIC_VALUE_TYPE_INT:   return OP_GET_INT_FROM_HEAP;
        case BASIC_VALUE_TYPE_FLOAT: return OP_GET_FLOAT_FROM_HEAP;

        case BASIC_VALUE_TYPE_STRING:
        case BASIC_VALUE_TYPE_ARRAY:
        case BASIC_VALUE_TYPE_MAP:
        case BASIC_VALUE_TYPE_FUNCTION:
        case BASIC_VALUE_TYPE_OBJECT:
            return OP_GET_ADDRESS_FROM_HEAP;

        case BASIC_VALUE_TYPE_VOID:
        case BASIC_VALUE_TYPE_PLAIN_STRUCTURE:
        case BASIC_VALUE_TYPE_REFERENCE_STRUCTURE:
        default:
            assert(false);
    }
}

OpCode getOpSetOnHeapForValueType(ValueType* value_type) {
    switch (value_type->basic_type) {
        case BASIC_VALUE_TYPE_BOOL:  return OP_SET_BYTE_ON_HEAP;
        case BASIC_VALUE_TYPE_INT:   return OP_SET_INT_ON_HEAP;
        case BASIC_VALUE_TYPE_FLOAT: return OP_SET_FLOAT_ON_HEAP;

        case BASIC_VALUE_TYPE_STRING:
        case BASIC_VALUE_TYPE_ARRAY:
        case BASIC_VALUE_TYPE_MAP:
        case BASIC_VALUE_TYPE_FUNCTION:
        case BASIC_VALUE_TYPE_OBJECT:
            return OP_SET_ADDRESS_ON_HEAP;

        case BASIC_VALUE_TYPE_VOID:
        case BASIC_VALUE_TYPE_PLAIN_STRUCTURE:
        case BASIC_VALUE_TYPE_REFERENCE_STRUCTURE:
        default:
            assert(false);
    }
}

