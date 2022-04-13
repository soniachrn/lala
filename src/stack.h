#ifndef lala_stack_h
#define lala_stack_h


#include <stdint.h>
#include <stdio.h>


// ┌────────┐
// │ Macros │
// └────────┘

#define STACK_INITIAL_SIZE 128
#define STACK_MAX_SIZE (1024 * 1024)  /* 1MB */


// ┌───────┐
// │ Types │
// └───────┘

typedef struct {
    uint8_t* stack;
    uint8_t* stack_top;
    size_t   size;
} Stack;


// ┌───────────────────────┐
// │ Function declarations │
// └───────────────────────┘

void initStack(Stack* stack);
void freeStack(Stack* stack);
void dumpStack(const Stack* stack);
void fdumpStack(FILE* out, const Stack* stack, int padding);

void pushByteOnStack(Stack* stack, uint8_t byte);
void pushIntOnStack(Stack* stack, int32_t value);
void pushFloatOnStack(Stack* stack, double value);

uint8_t popByteFromStack(Stack* stack);
int32_t popIntFromStack(Stack* stack);
double popFloatFromStack(Stack* stack);


#endif

