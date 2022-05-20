#include "stack.h"


#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "debug.h"


// ┌────────┐
// │ Macros │
// └────────┘

#define VALIDATE_STACK(stack)                        \
    (                                                \
        stack                            &&          \
        stack->stack                     &&          \
        stack->stack_top                 &&          \
        stack->capacity > 0              &&          \
        stack->stack_top >= stack->stack &&          \
        (size_t)(stack->stack_top - stack->stack) <= \
            stack->capacity                          \
    )

#define ASSERT_STACK(stack)                             \
    if (!VALIDATE_STACK(stack)) {                       \
        fprintf(stderr,                                 \
            "%s:%d, in %s:\nstack assertion failed.\n", \
            __FILENAME__,                               \
            __LINE__,                                   \
            __FUNCTION_NAME__                           \
        );                                              \
        fdumpStack(stderr, stack, 0);                   \
        exit(1);                                        \
    }


// ┌──────────────────────────────┐
// │ Static function declarations │
// └──────────────────────────────┘

static void ensureFreeSpace(Stack* stack, size_t space);
static void shrinkIfNeeded(Stack* stack);


// ┌──────────────────────────┐
// │ Function implementations │
// └──────────────────────────┘

void initStack(Stack* stack) {
    assert(stack);
    
    stack->stack = calloc(STACK_INITIAL_CAPACITY, 1);
    stack->stack_top = stack->stack;
    stack->capacity = STACK_INITIAL_CAPACITY;

    ASSERT_STACK(stack);
}

void freeStack(Stack* stack) {
    ASSERT_STACK(stack);

    free(stack->stack);
    stack->stack = NULL;
    stack->stack_top = NULL;
    stack->capacity = 0;
}

void dumpStack(const Stack* stack) {
    fdumpStack(stdout, stack, 0);
}

void fdumpStack(FILE* out, const Stack* stack, int padding) {
    assert(out);

#define printf(...)                                \
    {                                              \
        if (padding > 0) {                         \
            fprintf(out, "%*s", padding * 2, " "); \
        }                                          \
        fprintf(out, __VA_ARGS__);                 \
    }

#define MIN(a, b) (a < b ? a : b)

    if (!stack) {
        fprintf(out, "Stack *(NULL)\n");
    } else {
        fprintf(out, "Stack *(%p) %s {\n",
            (const void*)stack,
            VALIDATE_STACK(stack) ? "VALID" : "INVALID"
        );

        // capacity
        printf("  capacity = %zu\n", stack->capacity);

        // stack
        printf("  stack = *(%p) [\n", (const void*)stack->stack);
        for (size_t i = 0; i < MIN(stack->capacity, (size_t)128);) {
            printf("    %.2lx:", i);
            for (size_t j = 0; j < 8; ++j) {
                printf(" %02X", stack->stack[i]);
                i += 1;
            }
            printf("\n");
        }
        printf("    ...\n");
        printf("  ]\n");
        
        // stack_top
        size_t stack_top_index = (size_t)(stack->stack_top - stack->stack);
        printf("  stack_top = *(%p) stack + %zu [\n", (const void*)stack->stack_top, stack_top_index);
        size_t n = MIN(stack->capacity - (size_t)(stack->stack_top - stack->stack), (size_t)8);
        for (size_t i = 0; i < n;) {
            printf("    %.2lx:", stack_top_index + i);
            for (size_t j = 0; j < 8; ++j) {
                printf(" %02X", stack->stack_top[i]);
                i += 1;
            }
            printf("\n");
        }
        printf("    ...\n");
        printf("  ]\n");

        printf("}\n");
    }

#undef MIN
#undef printf
}

size_t stackSize(const Stack* stack) {
    ASSERT_STACK(stack);
    return (size_t)(stack->stack_top - stack->stack);
}

void popBytesFromStack(Stack* stack, size_t count) {
    ASSERT_STACK(stack);

    if (count > stackSize(stack)) {
        fprintf(
            stderr,
            "Trying to pop %lu bytes from a stack of size %lu.\n",
            count,
            stackSize(stack)
        );
        exit(1);
    }

    stack->stack_top -= count * sizeof(uint8_t);
    shrinkIfNeeded(stack);

    ASSERT_STACK(stack);
}

#define DEFINE_PUSH_POP_GET_SET_OPERATIONS(type_name, type)       \
    void push ## type_name ## OnStack(Stack* stack, type value) { \
        ASSERT_STACK(stack);                                      \
                                                                  \
        ensureFreeSpace(stack, sizeof(type));                     \
        *((type*)stack->stack_top) = value;                       \
        stack->stack_top += sizeof(type);                         \
                                                                  \
        ASSERT_STACK(stack);                                      \
    }                                                             \
                                                                  \
    type pop ## type_name ## FromStack(Stack* stack) {            \
        ASSERT_STACK(stack);                                      \
                                                                  \
                                                                  \
        if (sizeof(type) > stackSize(stack)) {                    \
            fprintf(                                              \
                stderr,                                           \
                "Tring to pop a " #type_name " (size %lu) "       \
                "from a stack of size %lu.\n",                    \
                sizeof(type),                                     \
                stackSize(stack)                                  \
            );                                                    \
            exit(1);                                              \
        }                                                         \
                                                                  \
        stack->stack_top -= sizeof(type);                         \
        type value = *((type*)stack->stack_top);                  \
        shrinkIfNeeded(stack);                                    \
                                                                  \
        ASSERT_STACK(stack);                                      \
        return value;                                             \
    }                                                             \
                                                                  \
    void set ## type_name ## OnStack(                             \
        Stack* stack,                                             \
        size_t address,                                           \
        type value                                                \
    ) {                                                           \
        ASSERT_STACK(stack);                                      \
                                                                  \
        if (address + sizeof(type) > stackSize(stack)) {          \
            /* TODO: error */                                     \
            fprintf(                                              \
                stderr,                                           \
                "Trying to set a " #type_name " (size %lu) "      \
                "with offset %lu in a stack of size %lu.\n",      \
                sizeof(type),                                     \
                address,                                          \
                stackSize(stack)                                  \
            );                                                    \
            exit(1);                                              \
        }                                                         \
                                                                  \
        *(type*)(stack->stack + address) = value;                 \
                                                                  \
        ASSERT_STACK(stack);                                      \
    }                                                             \
                                                                  \
    type get ## type_name ## FromStack(                           \
        const Stack* stack,                                       \
        size_t address                                            \
    ) {                                                           \
        ASSERT_STACK(stack);                                      \
                                                                  \
        if (address + sizeof(type) > stackSize(stack)) {          \
            fprintf(                                              \
                stderr,                                           \
                "Trying to get a " #type_name " (size %lu) "      \
                "with offset %lu from a stack of size %lu.\n",    \
                sizeof(type),                                     \
                address,                                          \
                stackSize(stack)                                  \
            );                                                    \
            exit(1);                                              \
        }                                                         \
                                                                  \
        ASSERT_STACK(stack);                                      \
        return *(type*)(stack->stack + address);                  \
    } static_assert(true, "require semicolon")

DEFINE_PUSH_POP_GET_SET_OPERATIONS(Byte,    uint8_t);
DEFINE_PUSH_POP_GET_SET_OPERATIONS(Int,     int32_t);
DEFINE_PUSH_POP_GET_SET_OPERATIONS(Float,   double);
DEFINE_PUSH_POP_GET_SET_OPERATIONS(Address, size_t);

#undef DEFINE_PUSH_POP_GET_SET_OPERATIONS


// ┌─────────────────────────────────┐
// │ Static function implementations │
// └─────────────────────────────────┘

static void reallocStack(Stack* stack, size_t new_capacity) {
    ASSERT_STACK(stack);

    if (new_capacity > STACK_MAX_CAPACITY) {
        assert(false);
    }

    size_t stack_top_offset = stackSize(stack);

    stack->stack = realloc(stack->stack, new_capacity);
    if (stack->stack == NULL) {
        assert(false);
    }

    stack->stack_top = stack->stack + stack_top_offset;
    stack->capacity = new_capacity;

    ASSERT_STACK(stack);
}

static void ensureFreeSpace(Stack* stack, size_t space_needed) {
    ASSERT_STACK(stack);

    size_t free_space = stack->capacity - stackSize(stack);
    if (free_space < space_needed) {
        reallocStack(stack, stack->capacity * 2);
    }

    ASSERT_STACK(stack);
}

static void shrinkIfNeeded(Stack* stack) {
    ASSERT_STACK(stack);

    size_t used = stackSize(stack);
    if (used < stack->capacity / 4 && used * 2 >= STACK_INITIAL_CAPACITY) {
        reallocStack(stack, used * 2);
    }

    ASSERT_STACK(stack);
}


#undef VALIDATE_STACK
#undef ASSERT_STACK

