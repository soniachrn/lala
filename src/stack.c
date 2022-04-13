#include "stack.h"


#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "debug.h"


// ┌────────┐
// │ Macros │
// └────────┘

#define VALIDATE_STACK(stack)                                    \
    (                                                            \
        stack                            &&                      \
        stack->stack                     &&                      \
        stack->stack_top                 &&                      \
        stack->size > 0                  &&                      \
        stack->stack_top >= stack->stack &&                      \
        (size_t)(stack->stack_top - stack->stack) <= stack->size \
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
    
    stack->stack = calloc(STACK_INITIAL_SIZE, 1);
    stack->stack_top = stack->stack;
    stack->size = STACK_INITIAL_SIZE;

    ASSERT_STACK(stack);
}

void freeStack(Stack* stack) {
    ASSERT_STACK(stack);

    free(stack->stack);
    stack->stack = NULL;
    stack->stack_top = NULL;
    stack->size = 0;
}

void dumpStack(const Stack* stack) {
    fdumpStack(stdout, stack, 0);
}

void fdumpStack(FILE* out, const Stack* stack, int padding) {
    assert(out);

#define printf(...)                             \
    {                                           \
        if (padding > 0) {                      \
            fprintf(out, "%*s", padding, "  "); \
        }                                       \
        fprintf(out, __VA_ARGS__);              \
    }

#define MIN(a, b) (a < b ? a : b)

    if (!stack) {
        printf("Stack *(NULL)\n");
    } else {
        printf("Stack *(%p) %s {\n",
            (const void*)stack,
            VALIDATE_STACK(stack) ? "VALID" : "INVALID"
        );

        // stack
        printf("  stack     = *(%p) [", (const void*)stack->stack);
        for (size_t i = 0; i < MIN(stack->size, (size_t)10); ++i) {
            fprintf(out, "0x%02X ", stack->stack[i]);
        }
        printf(" ...]\n");
        
        // stack_top
        printf("  stack_top = *(%p) [", (const void*)stack->stack_top);
        size_t n = MIN(stack->size - (size_t)(stack->stack_top - stack->stack), (size_t)10);
        for (size_t i = 0; i < n; ++i) {
            fprintf(out, "0x%02X ", stack->stack_top[i]);
        }
        printf(" ...]\n");

        // size
        printf("  size     = %zu\n", stack->size);
        printf("}\n");
    }

#undef MIN
#undef printf
}

void pushByteOnStack(Stack* stack, uint8_t byte) {
    ASSERT_STACK(stack);

    ensureFreeSpace(stack, 1);
    *(stack->stack_top++) = byte;

    ASSERT_STACK(stack);
}

void pushIntOnStack(Stack* stack, int32_t value) {
    ASSERT_STACK(stack);

    ensureFreeSpace(stack, sizeof(int32_t));
    *((int32_t*)stack->stack_top) = value;
    stack->stack_top += sizeof(int32_t);

    ASSERT_STACK(stack);
}

void pushFloatOnStack(Stack* stack, double value) {
    ASSERT_STACK(stack);

    ensureFreeSpace(stack, sizeof(double));
    *((double*)stack->stack_top) = value;
    stack->stack_top += sizeof(double);

    ASSERT_STACK(stack);
}

uint8_t popByteFromStack(Stack* stack) {
    ASSERT_STACK(stack);

    uint8_t value = *(--stack->stack_top);
    shrinkIfNeeded(stack);

    ASSERT_STACK(stack);
    return value;
}

int32_t popIntFromStack(Stack* stack) {
    ASSERT_STACK(stack);

    stack->stack_top -= sizeof(int32_t);
    int32_t value = *((int32_t*)stack->stack_top);
    shrinkIfNeeded(stack);

    ASSERT_STACK(stack);
    return value;
}

double popFloatFromStack(Stack* stack) {
    ASSERT_STACK(stack);

    stack->stack_top -= sizeof(double);
    double value = *((double*)stack->stack_top);
    shrinkIfNeeded(stack);

    ASSERT_STACK(stack);
    return value;
}


// ┌─────────────────────────────────┐
// │ Static function implementations │
// └─────────────────────────────────┘

static void reallocStack(Stack* stack, size_t new_size) {
    ASSERT_STACK(stack);

    if (new_size > STACK_MAX_SIZE) {
        // TODO: error
        assert(false);
    }

    size_t stack_top_offset = (size_t)(stack->stack_top - stack->stack);

    stack->stack = realloc(stack->stack, new_size);
    if (stack->stack == NULL) {
        // TODO: error
        assert(false);
    }

    stack->stack_top = stack->stack + stack_top_offset;
    stack->size = new_size;

    ASSERT_STACK(stack);
}

static void ensureFreeSpace(Stack* stack, size_t space_needed) {
    ASSERT_STACK(stack);

    size_t free_space = stack->size - (size_t)(stack->stack_top - stack->stack);
    if (free_space < space_needed) {
        reallocStack(stack, stack->size * 2);
    }

    ASSERT_STACK(stack);
}

static void shrinkIfNeeded(Stack* stack) {
    ASSERT_STACK(stack);

    size_t used = (size_t)(stack->stack_top - stack->stack);
    if (used < stack->size / 4 && used * 2 >= STACK_INITIAL_SIZE) {
        reallocStack(stack, used * 2);
    }

    ASSERT_STACK(stack);
}


#undef VALIDATE_STACK
#undef ASSERT_STACK

