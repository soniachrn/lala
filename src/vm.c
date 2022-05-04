#include "vm.h"


#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "debug.h"


// ┌────────┐
// │ Macros │
// └────────┘

#define VALIDATE_VM(vm)         \
    (                           \
        vm                   && \
        vm->source           && \
        vm->ip               && \
        vm->ip >= vm->source && \
        vm->call_frame          \
    )

#define ASSERT_VM(vm)                                \
    if (!VALIDATE_VM(vm)) {                          \
        fprintf(stderr,                              \
            "%s:%d, in %s:\nvm assertion failed.\n", \
            __FILENAME__,                            \
            __LINE__,                                \
            __FUNCTION_NAME__                        \
        );                                           \
        fdumpVM(stderr, vm, 0);                      \
        exit(1);                                     \
    }


// ┌──────────────────────────────┐
// │ Static function declarations │
// └──────────────────────────────┘

static void pushCallFrame(VM* vm);
static void popCallFrame(VM* vm);

static bool hasEnoughInputBytes(VM* vm, size_t expected_bytes);
static bool isAtEnd(VM* vm);

static uint8_t readByteFromSource(   VM* vm);
static int32_t readIntFromSource(    VM* vm);
static double  readFloatFromSource(  VM* vm);
static size_t  readAddressFromSource(VM* vm);


// ┌──────────────────────────┐
// │ Function implementations │
// └──────────────────────────┘

void initVM(
    VM* vm,
    uint8_t* source,
    size_t source_size,
    Constants* constants
) {
    assert(vm);

    vm->source_size = source_size;
    vm->source      = source;
    vm->ip          = source;
    vm->constants   = constants;

    initStack(&vm->stack);
    initHeap(&vm->heap);

    vm->call_frame = NULL;
    pushCallFrame(vm);

    ASSERT_VM(vm);
}

void freeVM(VM* vm) {
    ASSERT_VM(vm);

    popCallFrame(vm);

    vm->source = NULL;
    vm->ip = NULL;

    freeStack(&vm->stack);
    freeHeap(&vm->heap);

    assert(vm->call_frame == NULL);
}

void dumpVM(const VM* vm) {
    fdumpVM(stdout, vm, 0);
}

void fdumpVM(FILE* out, const VM* vm, int padding) {
    assert(out);

#define printf(...)                                \
    {                                              \
        if (padding > 0) {                         \
            fprintf(out, "%*s", padding * 2, " "); \
        }                                          \
        fprintf(out, __VA_ARGS__);                 \
    }

    (void)out; (void)vm; (void)padding;

#undef printf
}

void interpret(VM* vm) {
    ASSERT_VM(vm);

    // TODO check that ip didn't encounter '\0' when doint vm->ip +=

#define PUSH_BYTE(   value) pushByteOnStack(   &vm->stack, (value))
#define PUSH_INT(    value) pushIntOnStack(    &vm->stack, (value))
#define PUSH_FLOAT(  value) pushFloatOnStack(  &vm->stack, (value))
#define PUSH_ADDRESS(value) pushAddressOnStack(&vm->stack, (value))

#define POP_BYTE( )   popByteFromStack(   &vm->stack)
#define POP_INT(  )   popIntFromStack(    &vm->stack)
#define POP_FLOAT()   popFloatFromStack(  &vm->stack)
#define POP_ADDRESS() popAddressFromStack(&vm->stack)

    while (!isAtEnd(vm)) {
        switch ((OpCode)readByteFromSource(vm)) {
            // Stack
            case OP_PUSH_TRUE:    PUSH_BYTE(1); break;
            case OP_PUSH_FALSE:   PUSH_BYTE(0); break;
            case OP_PUSH_INT:     PUSH_INT(readIntFromSource(vm)); break;
            case OP_PUSH_FLOAT:   PUSH_FLOAT(readFloatFromSource(vm)); break;
            case OP_PUSH_ADDRESS: PUSH_ADDRESS(readAddressFromSource(vm)); break;

            case OP_POP_BYTE:     POP_BYTE(); break;
            case OP_POP_INT:      POP_INT(); break; 
            case OP_POP_FLOAT:    POP_FLOAT(); break;
            case OP_POP_ADDRESS:  POP_ADDRESS(); break;
            case OP_POP_BYTES:
                popBytesFromStack(&vm->stack, readAddressFromSource(vm));
                break;

            // Heap  
            case OP_LOAD_CONSTANT: {
                uint8_t constant_index = readByteFromSource(vm);
                assert(constant_index < vm->constants->count);
                Constant constant = vm->constants->constants[constant_index];
                Object* object = allocateObjectFromValue(
                    &vm->heap,
                    REFERENCE_RULE_PLAIN,
                    NULL,
                    constant.length,
                    constant.value
                );
                PUSH_ADDRESS((size_t)object);
                break;
            }

            case OP_DEFINE_ON_HEAP: {
                size_t length = readAddressFromSource(vm);
                ReferenceRule reference_rule = (ReferenceRule)readByteFromSource(vm);
                Object* object = allocateObjectFromValue(
                    &vm->heap,
                    reference_rule,
                    NULL,
                    length,
                    vm->stack.stack_top - length * sizeof(uint8_t)
                );
                popBytesFromStack(&vm->stack, length);
                PUSH_ADDRESS((size_t)object);
                break;
            }

            // Logical
            case OP_OR:  PUSH_BYTE(POP_BYTE() || POP_BYTE()); break;
            case OP_AND: PUSH_BYTE(POP_BYTE() && POP_BYTE()); break;
            case OP_NEGATE_BOOL: PUSH_BYTE(!POP_BYTE()); break;

            // Comparison
            case OP_EQUALS_BOOL:   PUSH_BYTE(POP_BYTE() == POP_BYTE()); break;
            case OP_EQUALS_INT:    PUSH_BYTE(POP_INT()  == POP_INT());  break;
            case OP_EQUALS_FLOAT:  PUSH_BYTE(fabs(POP_FLOAT() - POP_FLOAT()) < EPSILON); break;
            case OP_EQUALS_STRING: assert(false); // TODO: Not implemented yet

            // Inversed comparison sign here and later, 
            // because operand order on stack is inversed.
            case OP_LESS_INT:    PUSH_BYTE(POP_INT()   > POP_INT()); break;
            case OP_LESS_FLOAT:  PUSH_BYTE(POP_FLOAT() > POP_FLOAT()); break;
            case OP_LESS_STRING: assert(false); // TODO: Not implemented yet

            case OP_GREATER_INT:    PUSH_BYTE(POP_INT()   < POP_INT());   break;
            case OP_GREATER_FLOAT:  PUSH_BYTE(POP_FLOAT() < POP_FLOAT()); break;
            case OP_GREATER_STRING: assert(false); // TODO: Not implemented yet

            // Math
            case OP_ADD_INT:        PUSH_INT(  POP_INT()   + POP_INT());   break;
            case OP_ADD_FLOAT:      PUSH_FLOAT(POP_FLOAT() + POP_FLOAT()); break;

            case OP_MULTIPLY_INT:   PUSH_INT(  POP_INT()   * POP_INT());   break;
            case OP_MULTIPLY_FLOAT: PUSH_FLOAT(POP_FLOAT() * POP_FLOAT()); break;

            // TODO: make sure r isn't 0
            case OP_DIVIDE_INT: {
                int32_t r = POP_INT();
                int32_t l = POP_INT();
                PUSH_INT(l / r);
                break;
            }
            case OP_DIVIDE_FLOAT: {
                double r = POP_FLOAT();
                double l = POP_FLOAT();
                PUSH_FLOAT(l / r);
                break;
            }
            case OP_MODULO_INT: {
                int32_t r = POP_INT();
                int32_t l = POP_INT();
                PUSH_INT(l % r);
                break;
            }
            case OP_MODULO_FLOAT: assert(false); // TODO: Not implemented yet

            case OP_NEGATE_INT:   PUSH_INT(-POP_INT());     break;
            case OP_NEGATE_FLOAT: PUSH_FLOAT(-POP_FLOAT()); break;

            // String
            case OP_CONCATENATE: {
                Object* r_address = (Object*)POP_ADDRESS();
                Object* l_address = (Object*)POP_ADDRESS();

                Object* object = allocateEmptyObject(
                    &vm->heap,
                    REFERENCE_RULE_PLAIN,
                    NULL,
                    l_address->size + r_address->size
                );
                memcpy(object->value, l_address->value, l_address->size);
                memcpy(object->value + l_address->size, r_address->value, r_address->size);

                PUSH_ADDRESS((size_t)object);
                break;
            }

            // Cast
            case OP_CAST_FLOAT_TO_INT: PUSH_INT((int32_t)POP_FLOAT()); break;
            case OP_CAST_INT_TO_FLOAT: PUSH_FLOAT((double)POP_INT()); break;
            case OP_CAST_BOOL_TO_STRING:
                PUSH_ADDRESS((size_t)(POP_BYTE() ? &OBJECT_STRING_TRUE : &OBJECT_STRING_FALSE));
                break;
            case OP_CAST_INT_TO_STRING: {
                char buffer[128];
                int length = snprintf(buffer, 128, "%d", POP_INT());

                Object* object = allocateObjectFromValue(
                    &vm->heap,
                    REFERENCE_RULE_PLAIN,
                    NULL,
                    (size_t)length,
                    (uint8_t*)buffer
                );
                PUSH_ADDRESS((size_t)object);
                break;
            }
            case OP_CAST_FLOAT_TO_STRING: {
                char buffer[128];
                int length = snprintf(buffer, 128, "%g", POP_FLOAT());

                Object* object = allocateObjectFromValue(
                    &vm->heap,
                    REFERENCE_RULE_PLAIN,
                    NULL,
                    (size_t)length,
                    (uint8_t*)buffer
                );
                PUSH_ADDRESS((size_t)object);
                break;
            }

#define GET_FROM_STACK_OP(type_name, local)                  \
    push ## type_name ## OnStack(                            \
        &vm->stack,                                          \
        get ## type_name ## FromStack(                       \
            &vm->stack,                                      \
            (                                                \
                (local ? vm->call_frame->stack_offset : 0) + \
                readAddressFromSource(vm)                    \
            )                                                \
        )                                                    \
    )

#define SET_ON_STACK_OP(type_name, local)                \
    set ## type_name ## OnStack(                         \
        &vm->stack,                                      \
        (                                                \
            (local ? vm->call_frame->stack_offset : 0) + \
            readAddressFromSource(vm)                    \
        ),                                               \
        pop ## type_name ## FromStack(&vm->stack)        \
    )

            // Variables

            case OP_GET_LOCAL_BYTE:    GET_FROM_STACK_OP(Byte,    true); break;
            case OP_GET_LOCAL_INT:     GET_FROM_STACK_OP(Int,     true); break;
            case OP_GET_LOCAL_FLOAT:   GET_FROM_STACK_OP(Float,   true); break;
            case OP_GET_LOCAL_ADDRESS: GET_FROM_STACK_OP(Address, true); break;

            case OP_SET_LOCAL_BYTE:    SET_ON_STACK_OP(Byte,    true); break;
            case OP_SET_LOCAL_INT:     SET_ON_STACK_OP(Int,     true); break;
            case OP_SET_LOCAL_FLOAT:   SET_ON_STACK_OP(Float,   true); break;
            case OP_SET_LOCAL_ADDRESS: SET_ON_STACK_OP(Address, true); break;

            case OP_GET_GLOBAL_BYTE:    GET_FROM_STACK_OP(Byte,    false); break;
            case OP_GET_GLOBAL_INT:     GET_FROM_STACK_OP(Int,     false); break;
            case OP_GET_GLOBAL_FLOAT:   GET_FROM_STACK_OP(Float,   false); break;
            case OP_GET_GLOBAL_ADDRESS: GET_FROM_STACK_OP(Address, false); break;

            case OP_SET_GLOBAL_BYTE:    SET_ON_STACK_OP(Byte,    false); break;
            case OP_SET_GLOBAL_INT:     SET_ON_STACK_OP(Int,     false); break;
            case OP_SET_GLOBAL_FLOAT:   SET_ON_STACK_OP(Float,   false); break;
            case OP_SET_GLOBAL_ADDRESS: SET_ON_STACK_OP(Address, false); break;

#undef SET_ON_STACK_OP
#undef GET_FROM_STACK_OP

            // Print
            case OP_PRINT_BOOL:
                printf("%s\n", POP_BYTE() ? "true" : "false");
                break;
            case OP_PRINT_INT:
                printf("%d\n", POP_INT());
                break;
            case OP_PRINT_FLOAT:
                printf("%g\n", POP_FLOAT());
                break;
            case OP_PRINT_STRING: {
                Object* object = (Object*)POP_ADDRESS();
                printf("%.*s\n", (int)object->size, object->value);
                break;
            }

            // Jump
            case OP_JUMP:
                vm->ip = vm->source + readAddressFromSource(vm);
                break;
            case OP_JUMP_IF_TRUE: {
                size_t address = readAddressFromSource(vm);
                if (POP_BYTE()) {
                    vm->ip = vm->source + address;
                }
                break;
            }
            case OP_JUMP_IF_FALSE: {
                size_t address = readAddressFromSource(vm);
                if (!POP_BYTE()) {
                    vm->ip = vm->source + address;
                }
                break;
            }

            // Functions
            case OP_CALL: {
                size_t offset_from_call_frame_start = readAddressFromSource(vm);

                pushCallFrame(vm);
                assert(offset_from_call_frame_start <= vm->call_frame->stack_offset);
                vm->call_frame->stack_offset -= offset_from_call_frame_start;

                size_t function_address = getAddressFromStack(
                    &vm->stack,
                    vm->call_frame->stack_offset + FUNCTION_ADDRESS_POSITION_IN_CALL_FRAME
                );
                vm->ip = vm->source + function_address;
                break;
            }

            case OP_RETURN_VOID: {
                size_t return_address = getAddressFromStack(
                    &vm->stack,
                    vm->call_frame->stack_offset + RETURN_ADDRESS_POSITION_IN_CALL_FRAME
                );

                popCallFrame(vm);

                vm->ip = vm->source + return_address;
                break;                          
            }

            case OP_RETURN_BYTE: {
                uint8_t return_value = POP_BYTE();
                size_t return_address = getAddressFromStack(
                    &vm->stack,
                    vm->call_frame->stack_offset + RETURN_ADDRESS_POSITION_IN_CALL_FRAME
                );

                popCallFrame(vm);

                PUSH_BYTE(return_value);
                vm->ip = vm->source + return_address;
                break;
            }

            case OP_RETURN_INT: {
                int32_t return_value = POP_INT();
                size_t return_address = getAddressFromStack(
                    &vm->stack,
                    vm->call_frame->stack_offset + RETURN_ADDRESS_POSITION_IN_CALL_FRAME
                );

                popCallFrame(vm);

                PUSH_INT(return_value);
                vm->ip = vm->source + return_address;
                break;
            }

            case OP_RETURN_FLOAT: {
                double return_value = POP_FLOAT();
                size_t return_address = getAddressFromStack(
                    &vm->stack,
                    vm->call_frame->stack_offset + RETURN_ADDRESS_POSITION_IN_CALL_FRAME
                );

                popCallFrame(vm);

                PUSH_FLOAT(return_value);
                vm->ip = vm->source + return_address;
                break;
            }

            case OP_RETURN_ADDRESS: {
                size_t return_value = POP_ADDRESS();
                size_t return_address = getAddressFromStack(
                    &vm->stack,
                    vm->call_frame->stack_offset + RETURN_ADDRESS_POSITION_IN_CALL_FRAME
                );

                popCallFrame(vm);

                PUSH_ADDRESS(return_value);
                vm->ip = vm->source + return_address;
                break;
            }

            // Array
            case OP_SUBSCRIPT_BYTE: {
                int32_t index = POP_INT();
                if (index < 0) {
                    fprintf(stderr, "negative array index\n");
                    exit(1);
                }
                Object* array_object = (Object*)POP_ADDRESS();
                if ((size_t)index + sizeof(uint8_t) > array_object->size) {
                    fprintf(stderr, "array out of bounds\n");
                    exit(1);
                }
                PUSH_BYTE(((uint8_t*)array_object->value)[index]);
                break;
            }

            case OP_SUBSCRIPT_INT: {
                int32_t index = POP_INT();
                if (index < 0) {
                    fprintf(stderr, "negative array index\n");
                    exit(1);
                }
                Object* array_object = (Object*)POP_ADDRESS();
                if ((size_t)index + sizeof(int32_t) > array_object->size) {
                    fprintf(stderr, "array out of bounds\n");
                    exit(1);
                }
                PUSH_INT(((int32_t*)array_object->value)[index]);
                break;
            }

            case OP_SUBSCRIPT_FLOAT: {
                int32_t index = POP_INT();
                if (index < 0) {
                    fprintf(stderr, "negative array index\n");
                    exit(1);
                }
                Object* array_object = (Object*)POP_ADDRESS();
                if ((size_t)index + sizeof(double) > array_object->size) {
                    fprintf(stderr, "array out of bounds\n");
                    exit(1);
                }
                PUSH_FLOAT(((double*)array_object->value)[index]);
                break;
            }

            case OP_SUBSCRIPT_ADDRESS: {
                int32_t index = POP_INT();
                if (index < 0) {
                    fprintf(stderr, "negative array index\n");
                    exit(1);
                }
                Object* array_object = (Object*)POP_ADDRESS();
                if ((size_t)index + sizeof(size_t) > array_object->size) {
                    fprintf(stderr, "array out of bounds\n");
                    exit(1);
                }
                PUSH_ADDRESS(((size_t*)array_object->value)[index]);
                break;
            }

            default: assert(false); // TODO: error
        }
    }

#undef POP_ADDRESS
#undef POP_FLOAT
#undef POP_INT
#undef POP_BYTE

#undef PUSH_ADDRESS
#undef PUSH_FLOAT
#undef PUSH_INT
#undef PUSH_BYTE

    ASSERT_VM(vm);
}


// ┌─────────────────────────────────┐
// │ Static function implementations │
// └─────────────────────────────────┘

static void pushCallFrame(VM* vm) {
    assert(vm);

    CallFrame* new_call_frame = calloc(1, sizeof(CallFrame));
    new_call_frame->parent = vm->call_frame;
    new_call_frame->stack_offset = stackSize(&vm->stack);
    vm->call_frame = new_call_frame;

    ASSERT_VM(vm);
}

static void popCallFrame(VM* vm) {
    ASSERT_VM(vm);
    
    popBytesFromStack(&vm->stack, stackSize(&vm->stack) - vm->call_frame->stack_offset);

    CallFrame* old_call_frame = vm->call_frame;
    vm->call_frame = vm->call_frame->parent;
    free(old_call_frame);
}

static bool hasEnoughInputBytes(VM* vm, size_t expected_bytes) {
    ASSERT_VM(vm);
    return vm->source_size - (size_t)(vm->ip - vm->source) >= expected_bytes;
}

static bool isAtEnd(VM* vm) {
    ASSERT_VM(vm);
    return (size_t)(vm->ip - vm->source) >= vm->source_size;
}

static uint8_t readByteFromSource(VM* vm) {
    ASSERT_VM(vm);

    if (isAtEnd(vm)) {
        fprintf(stderr, "Runtime error: expected a byte argument to command %02X, but got end of program.", *(vm->ip - 1));
        exit(1);
    }

    return *vm->ip++;
}

static int32_t readIntFromSource(VM* vm) {
    ASSERT_VM(vm);

    if (!hasEnoughInputBytes(vm, sizeof(uint32_t))) {
        fprintf(stderr, "Runtime error: expected an int argument to command %02X, but got end of program.", *(vm->ip - 1));
        exit(1);
    }

    int32_t value = *(int32_t*)vm->ip;
    vm->ip += sizeof(int32_t);
    return value;
}

static double readFloatFromSource(VM* vm) {
    ASSERT_VM(vm);

    if (!hasEnoughInputBytes(vm, sizeof(double))) {
        fprintf(stderr, "Runtime error: expected a float argument to command %02X, but got end of program.", *(vm->ip - 1));
        exit(1);
    }

    double value = *(double*)vm->ip;
    vm->ip += sizeof(double);
    return value;
}

static size_t readAddressFromSource(VM* vm) {
    ASSERT_VM(vm);

    if (!hasEnoughInputBytes(vm, sizeof(size_t))) {
        fprintf(stderr, "Runtime error: expected an address argument to command %02X, but got end of program.", *(vm->ip - 1));
        exit(1);
    }

    size_t value = *(size_t*)vm->ip;
    vm->ip += sizeof(size_t);
    return value;
}


#undef ASSERT_VM
#undef VALIDATE_VM

