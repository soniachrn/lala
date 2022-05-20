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

#define error(vm, ...)                                      \
    {                                                       \
        fprintf(                                            \
            stderr,                                         \
            "Runtime error at instruction '%s' at 0x%lx:\n", \
            opCodeName((OpCode)*vm->current_op_code),       \
            (size_t)(vm->current_op_code - vm->source)      \
        );                                                  \
        fprintf(stderr, __VA_ARGS__);                       \
        fprintf(stderr, "\n");                              \
        exit(1);                                            \
    }

#define notImplemented(vm)                           \
    {                                                \
        error(                                       \
            vm,                                      \
            "%s is not implemented yet.",            \
            opCodeName((OpCode)*vm->current_op_code) \
        );                                           \
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

    vm->source_size     = source_size;
    vm->source          = source;
    vm->current_op_code = NULL;
    vm->ip              = source;
    vm->constants       = constants;

    initStack(&vm->stack);
    initHeap(&vm->heap);

    vm->call_frame = NULL;
    pushCallFrame(vm);

    initStack(&vm->stack_references_positions);

    ASSERT_VM(vm);
}

void freeVM(VM* vm) {
    ASSERT_VM(vm);

    popCallFrame(vm);

    vm->source          = NULL;
    vm->current_op_code = NULL;
    vm->ip              = NULL;

    freeStack(&vm->stack);
    freeHeap(&vm->heap);

    assert(vm->call_frame == NULL);

    freeStack(&vm->stack_references_positions);
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

    if (!vm) {
        fprintf(out, "VM *(NULL)\n");
    } else {
        fprintf(out, "VM *(%p) %s {\n",
            (const void*)vm,
            VALIDATE_VM(vm) ? "VALID" : "INVALID"
        );

        printf("  source_size = %ld\n", vm->source_size);
        printf("  source = ");
        if (vm->source) {
            fprintf(out, "*(%p)\n", (void*)vm->source);
        } else {
            fprintf(out, "*(NULL)\n");
        }
        printf("  ip = ");
        if (vm->ip) {
            fprintf(out, "*(%p)\n", (void*)vm->ip);
        } else {
            fprintf(out, "*(NULL)\n");
        }
        printf("  constants = ");
        if (vm->constants) {
            fprintf(out, "*(%p)\n", (void*)vm->constants);
        } else {
            fprintf(out, "*(NULL)\n");
        }
        printf("  call_frame = ");
        if (vm->call_frame) {
            fprintf(out, "*(%p)\n", (void*)vm->call_frame);
        } else {
            fprintf(out, "*(NULL)\n");
        }
        printf("  stack = ");
        fdumpStack(out, &vm->stack, padding + 1);
        printf("  stack_referenes_positions = [\n");
        for (
            size_t i = 0;
            i < stackSize(&vm->stack_references_positions) / sizeof(size_t);
            ++i
        ) {
            printf(
                "    [%ld] = %ld,\n", 
                i,
                getAddressFromStack(
                    &vm->stack_references_positions,
                    i * sizeof(size_t)
                )
            );
        }
        printf("  ]\n");
        printf("  heap = ");
        fdumpHeap(out, &vm->heap, padding + 1);
        printf("}\n");
    }

#undef printf
}

void interpret(VM* vm) {
    ASSERT_VM(vm);

#define CLEAN_STACK_REFERENCES()                                                \
{                                                                               \
    while (                                                                     \
        stackSize(&vm->stack_references_positions) > 0 &&                       \
        *(size_t*)(vm->stack_references_positions.stack_top - sizeof(size_t)) > \
        stackSize(&vm->stack) - sizeof(size_t)                                  \
    ) {                                                                         \
        popAddressFromStack(&vm->stack_references_positions);                   \
    }                                                                           \
}


#define PUSH_BYTE(         value) pushByteOnStack(   &vm->stack, (value))
#define PUSH_INT(          value) pushIntOnStack(    &vm->stack, (value))
#define PUSH_FLOAT(        value) pushFloatOnStack(  &vm->stack, (value))
#define PUSH_PLAIN_ADDRESS(value) pushAddressOnStack(&vm->stack, (value))
#define PUSH_REF_ADDRESS(value)              \
    {                                        \
        pushAddressOnStack(                  \
            &vm->stack_references_positions, \
            stackSize(&vm->stack)            \
        );                                   \
        PUSH_PLAIN_ADDRESS(value);           \
    }

#define POP_BYTE()                                      \
    ({                                                  \
        uint8_t value = popByteFromStack(&vm->stack);   \
        CLEAN_STACK_REFERENCES();                       \
        value;                                          \
    })

#define POP_INT()                                       \
    ({                                                  \
        int32_t value = popIntFromStack(&vm->stack);    \
        CLEAN_STACK_REFERENCES();                       \
        value;                                          \
    })

#define POP_FLOAT()                                     \
    ({                                                  \
        double value = popFloatFromStack(&vm->stack);   \
        CLEAN_STACK_REFERENCES();                       \
        value;                                          \
    })

#define POP_ADDRESS()                                   \
    ({                                                  \
        size_t value = popAddressFromStack(&vm->stack); \
        CLEAN_STACK_REFERENCES();                       \
        value;                                          \
    })


    while (!isAtEnd(vm)) {
        vm->current_op_code = vm->ip;
        switch ((OpCode)readByteFromSource(vm)) {
            // Stack
            case OP_PUSH_TRUE:    PUSH_BYTE(1); break;
            case OP_PUSH_FALSE:   PUSH_BYTE(0); break;
            case OP_PUSH_BYTE:    PUSH_BYTE(readByteFromSource(vm)); break;
            case OP_PUSH_INT:     PUSH_INT(readIntFromSource(vm)); break;
            case OP_PUSH_FLOAT:   PUSH_FLOAT(readFloatFromSource(vm)); break;
            case OP_PUSH_ADDRESS: PUSH_PLAIN_ADDRESS(readAddressFromSource(vm)); break;

            case OP_POP_BYTE:     POP_BYTE(); break;
            case OP_POP_INT:      POP_INT(); break; 
            case OP_POP_FLOAT:    POP_FLOAT(); break;
            case OP_POP_ADDRESS:  POP_ADDRESS(); break;
            case OP_POP_BYTES:
                popBytesFromStack(&vm->stack, readAddressFromSource(vm));
                CLEAN_STACK_REFERENCES();
                break;

            // Heap  
            case OP_LOAD_CONSTANT: {
                uint8_t constant_index = readByteFromSource(vm);
                if (constant_index >= vm->constants->count) {
                    error(
                        vm,
                        "Trying to access constant %d, whereas there are only %d"
                        "constants declared in the constants section.",
                        constant_index,
                        vm->constants->count
                    );
                }

                Constant constant = vm->constants->constants[constant_index];
                Object* object = allocateObjectFromValue(
                    &vm->heap,
                    &vm->stack,
                    &vm->stack_references_positions,
                    REFERENCE_RULE_PLAIN,
                    NULL,
                    constant.length,
                    constant.value
                );
                PUSH_REF_ADDRESS((size_t)object);
                break;
            }

            case OP_DEFINE_ON_HEAP: {
                size_t length = readAddressFromSource(vm);
                ReferenceRule reference_rule = (ReferenceRule)readByteFromSource(vm);
                Object* custom_reference_rule = NULL;
                if (reference_rule == REFERENCE_RULE_CUSTOM) {
                    custom_reference_rule = (Object*)POP_ADDRESS();
                    // Mark in order for custom reference rule to not be deleted by gc before object allocation.
                    custom_reference_rule->marked = true;
                }
                Object* object = allocateObjectFromValue(
                    &vm->heap,
                    &vm->stack,
                    &vm->stack_references_positions,
                    reference_rule,
                    custom_reference_rule,
                    length,
                    vm->stack.stack_top - length * sizeof(uint8_t)
                );
                popBytesFromStack(&vm->stack, length);
                CLEAN_STACK_REFERENCES();
                PUSH_REF_ADDRESS((size_t)object);
                break;
            }

#define GET_FROM_HEAP_OP(type, push)                                          \
    {                                                                         \
        Object* object = (Object*)POP_ADDRESS();                              \
        size_t offset = readAddressFromSource(vm);                            \
        if (offset + sizeof(type) > object->size) {                           \
            error(                                                            \
                vm,                                                           \
                "Trying to read %lu bytes from a heap object at offset %lu, " \
                "but the object is only %lu bytes long.",                     \
                sizeof(type),                                                 \
                offset,                                                       \
                object->size                                                  \
            );                                                                \
        }                                                                     \
        push(*(type*)(object->value + offset));                               \
    }

            case OP_GET_BYTE_FROM_HEAP:    GET_FROM_HEAP_OP(uint8_t, PUSH_BYTE);        break;
            case OP_GET_INT_FROM_HEAP:     GET_FROM_HEAP_OP(int32_t, PUSH_INT);         break;
            case OP_GET_FLOAT_FROM_HEAP:   GET_FROM_HEAP_OP(double,  PUSH_FLOAT);       break;
            case OP_GET_ADDRESS_FROM_HEAP: GET_FROM_HEAP_OP(size_t,  PUSH_REF_ADDRESS); break;

#undef GET_FROM_HEAP_OP

#define SET_ON_HEAP_OP(type, pop)                                          \
    {                                                                      \
        type value = pop();                                                \
        Object* object = (Object*)POP_ADDRESS();                           \
        size_t offset = readAddressFromSource(vm);                         \
        if (offset + sizeof(type) > object->size) {                        \
            error(                                                         \
                vm,                                                        \
                "Trying to set %lu bytes in a heap object at offset %lu, " \
                "but the object is only %lu bytes long.",                  \
                sizeof(type),                                              \
                offset,                                                    \
                object->size                                               \
            );                                                             \
        }                                                                  \
        *(type*)(object->value + offset) = value;                          \
    }

            case OP_SET_BYTE_ON_HEAP:    SET_ON_HEAP_OP(uint8_t, POP_BYTE);    break;
            case OP_SET_INT_ON_HEAP:     SET_ON_HEAP_OP(int32_t, POP_INT);     break;
            case OP_SET_FLOAT_ON_HEAP:   SET_ON_HEAP_OP(double,  POP_FLOAT);   break;
            case OP_SET_ADDRESS_ON_HEAP: SET_ON_HEAP_OP(size_t,  POP_ADDRESS); break;

#undef SET_ON_HEAP_OP

            // Logical
            case OP_OR:  PUSH_BYTE(POP_BYTE() || POP_BYTE()); break;
            case OP_AND: PUSH_BYTE(POP_BYTE() && POP_BYTE()); break;
            case OP_NEGATE_BOOL: PUSH_BYTE(!POP_BYTE()); break;

            // Comparison
            case OP_EQUALS_BOOL:   PUSH_BYTE(POP_BYTE() == POP_BYTE()); break;
            case OP_EQUALS_INT:    PUSH_BYTE(POP_INT()  == POP_INT());  break;
            case OP_EQUALS_FLOAT:  PUSH_BYTE(fabs(POP_FLOAT() - POP_FLOAT()) < EPSILON); break;
            case OP_EQUALS_STRING: notImplemented(vm); break;

            // Inversed comparison sign here and later, 
            // because operand order on stack is inversed.
            case OP_LESS_INT:    PUSH_BYTE(POP_INT()   > POP_INT()); break;
            case OP_LESS_FLOAT:  PUSH_BYTE(POP_FLOAT() > POP_FLOAT()); break;
            case OP_LESS_STRING: notImplemented(vm); break;

            case OP_GREATER_INT:    PUSH_BYTE(POP_INT()   < POP_INT());   break;
            case OP_GREATER_FLOAT:  PUSH_BYTE(POP_FLOAT() < POP_FLOAT()); break;
            case OP_GREATER_STRING: notImplemented(vm); break;

            // Math
            case OP_ADD_INT:        PUSH_INT(  POP_INT()   + POP_INT());   break;
            case OP_ADD_FLOAT:      PUSH_FLOAT(POP_FLOAT() + POP_FLOAT()); break;

            case OP_MULTIPLY_INT:   PUSH_INT(  POP_INT()   * POP_INT());   break;
            case OP_MULTIPLY_FLOAT: PUSH_FLOAT(POP_FLOAT() * POP_FLOAT()); break;

            // TODO: make sure r isn't 0
            case OP_DIVIDE_INT: {
                int32_t r = POP_INT();
                int32_t l = POP_INT();
                if (r == 0) {
                    error(
                        vm,
                        "Division right operand is zero."
                    );
                }
                PUSH_INT(l / r);
                break;
            }
            case OP_DIVIDE_FLOAT: {
                double r = POP_FLOAT();
                double l = POP_FLOAT();
                if (fabs(r) < EPSILON) {
                    error(
                        vm,
                        "Division right operand is zero."
                    );
                }
                PUSH_FLOAT(l / r);
                break;
            }
            case OP_MODULO_INT: {
                int32_t r = POP_INT();
                int32_t l = POP_INT();
                if (r == 0) {
                    error(
                        vm,
                        "Modulo right operand is zero."
                    );
                }
                PUSH_INT(l % r);
                break;
            }
            case OP_MODULO_FLOAT: notImplemented(vm); break;

            case OP_NEGATE_INT:   PUSH_INT(-POP_INT());     break;
            case OP_NEGATE_FLOAT: PUSH_FLOAT(-POP_FLOAT()); break;

            // String
            case OP_CONCATENATE: {
                Object* r_address = (Object*)POP_ADDRESS();
                Object* l_address = (Object*)POP_ADDRESS();
                // Mark operands in order for them to not be deleted by gc before concatenation result allocation.
                r_address->marked = true;
                l_address->marked = true;

                Object* object = allocateEmptyObject(
                    &vm->heap,
                    &vm->stack,
                    &vm->stack_references_positions,
                    REFERENCE_RULE_PLAIN,
                    NULL,
                    l_address->size + r_address->size
                );
                memcpy(object->value, l_address->value, l_address->size);
                memcpy(object->value + l_address->size, r_address->value, r_address->size);

                PUSH_REF_ADDRESS((size_t)object);
                break;
            }

            // Cast
            case OP_CAST_FLOAT_TO_INT: PUSH_INT((int32_t)POP_FLOAT()); break;
            case OP_CAST_INT_TO_FLOAT: PUSH_FLOAT((double)POP_INT()); break;
            case OP_CAST_BOOL_TO_STRING:
                PUSH_REF_ADDRESS((size_t)(POP_BYTE() ? &OBJECT_STRING_TRUE : &OBJECT_STRING_FALSE));
                break;

#define CAST_NUMBER_TO_STRING_OP(format, value)             \
    {                                                       \
        char buffer[128];                                   \
        int length = snprintf(buffer, 128, format, value);  \
                                                            \
        Object* object = allocateObjectFromValue(           \
            &vm->heap,                                      \
            &vm->stack,                                     \
            &vm->stack_references_positions,                \
            REFERENCE_RULE_PLAIN,                           \
            NULL,                                           \
            (size_t)length,                                 \
            (uint8_t*)buffer                                \
        );                                                  \
        PUSH_REF_ADDRESS((size_t)object);                   \
    }

            case OP_CAST_INT_TO_STRING:   CAST_NUMBER_TO_STRING_OP("%d", POP_INT());   break;
            case OP_CAST_FLOAT_TO_STRING: CAST_NUMBER_TO_STRING_OP("%g", POP_FLOAT()); break;

#undef CAST_NUMBER_TO_STRING_OP

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
                if (offset_from_call_frame_start > vm->call_frame->stack_offset) {
                    error(
                        vm,
                        "In a call instruction, got offset from call frame start "
                        "argument = %lu, whereas the stack size is only %lu.",
                        offset_from_call_frame_start,
                        vm->call_frame->stack_offset
                    );
                }
                vm->call_frame->stack_offset -= offset_from_call_frame_start;

                Object* function_object = (Object*)getAddressFromStack(
                    &vm->stack,
                    vm->call_frame->stack_offset + FUNCTION_ADDRESS_POSITION_IN_CALL_FRAME
                );
                if (function_object->size != sizeof(size_t)) {
                    error(
                        vm,
                        "In a call instruction, the function object size is %lu,"
                        "expected to be %lu.",
                        function_object->size,
                        sizeof(size_t)
                    );
                }
                size_t function_address = *(size_t*)function_object->value;
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

#define RETURN_OP(type, pop, push)                                                \
    {                                                                             \
        type return_value = pop();                                                \
        size_t return_address = getAddressFromStack(                              \
            &vm->stack,                                                           \
            vm->call_frame->stack_offset + RETURN_ADDRESS_POSITION_IN_CALL_FRAME  \
        );                                                                        \
                                                                                  \
        popCallFrame(vm);                                                         \
                                                                                  \
        push(return_value);                                                       \
        vm->ip = vm->source + return_address;                                     \
    }

            case OP_RETURN_BYTE:    RETURN_OP(uint8_t, POP_BYTE,    PUSH_BYTE);        break;
            case OP_RETURN_INT:     RETURN_OP(int32_t, POP_INT,     PUSH_INT);         break;
            case OP_RETURN_FLOAT:   RETURN_OP(double,  POP_FLOAT,   PUSH_FLOAT);       break;
            case OP_RETURN_ADDRESS: RETURN_OP(size_t,  POP_ADDRESS, PUSH_REF_ADDRESS); break;

#undef RETURN_OP

#define SUBSCRIPT_OP(type, push)                                 \
    {                                                            \
        int32_t index = POP_INT();                               \
        if (index < 0) {                                         \
            fprintf(stderr, "negative array index\n");           \
            exit(1);                                             \
        }                                                        \
        Object* array_object = (Object*)POP_ADDRESS();           \
        if ((size_t)index + sizeof(type) > array_object->size) { \
            fprintf(stderr, "array out of bounds\n");            \
            exit(1);                                             \
        }                                                        \
        push(((type*)array_object->value)[index]);               \
    }

            // Array
            case OP_SUBSCRIPT_BYTE:    SUBSCRIPT_OP(uint8_t, PUSH_BYTE);        break;
            case OP_SUBSCRIPT_INT:     SUBSCRIPT_OP(int32_t, PUSH_INT);         break;
            case OP_SUBSCRIPT_FLOAT:   SUBSCRIPT_OP(double,  PUSH_FLOAT);       break;
            case OP_SUBSCRIPT_ADDRESS: SUBSCRIPT_OP(size_t,  PUSH_REF_ADDRESS); break;

#undef SUBSCRIPT_OP

            default: error(vm, "Invalid instruction."); break;
        }
    }

#undef POP_ADDRESS
#undef POP_FLOAT
#undef POP_INT
#undef POP_BYTE

#undef PUSH_REF_ADDRESS
#undef PUSH_PLAIN_ADDRESS
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
    CLEAN_STACK_REFERENCES();

    CallFrame* old_call_frame = vm->call_frame;
    vm->call_frame = vm->call_frame->parent;
    free(old_call_frame);
}

#undef CLEAN_STACK_REFERENCES

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
        error(vm, "Expected a byte argument, but got end of program.");
    }

    return *vm->ip++;
}

static int32_t readIntFromSource(VM* vm) {
    ASSERT_VM(vm);

    if (!hasEnoughInputBytes(vm, sizeof(uint32_t))) {
        error(vm, "Expected an int argument, but got end of program.");
    }

    int32_t value = *(int32_t*)vm->ip;
    vm->ip += sizeof(int32_t);
    return value;
}

static double readFloatFromSource(VM* vm) {
    ASSERT_VM(vm);

    if (!hasEnoughInputBytes(vm, sizeof(double))) {
        error(vm, "Expected a float argument, but got end of program.");
    }

    double value = *(double*)vm->ip;
    vm->ip += sizeof(double);
    return value;
}

static size_t readAddressFromSource(VM* vm) {
    ASSERT_VM(vm);

    if (!hasEnoughInputBytes(vm, sizeof(size_t))) {
        error(vm, "Expected an address argument, but got end of program.");
    }

    size_t value = *(size_t*)vm->ip;
    vm->ip += sizeof(size_t);
    return value;
}


#undef notImplemented
#undef error

#undef ASSERT_VM
#undef VALIDATE_VM

