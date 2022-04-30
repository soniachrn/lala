#ifndef lala_stack_h
#define lala_stack_h


#include <stdint.h>
#include <stdio.h>


// ┌────────┐
// │ Macros │
// └────────┘

#define STACK_INITIAL_CAPACITY 128
#define STACK_MAX_CAPACITY (1024 * 1024)  /* 1MB */


// ┌───────┐
// │ Types │
// └───────┘

typedef struct {
    uint8_t* stack;
    uint8_t* stack_top;
    size_t   capacity;
} Stack;


// ┌───────────────────────┐
// │ Function declarations │
// └───────────────────────┘

void initStack(Stack* stack);
void freeStack(Stack* stack);
void dumpStack(const Stack* stack);
void fdumpStack(FILE* out, const Stack* stack, int padding);

size_t stackSize(const Stack* stack);

void pushByteOnStack(   Stack* stack, uint8_t value);
void pushIntOnStack(    Stack* stack, int32_t value);
void pushFloatOnStack(  Stack* stack, double  value);
void pushAddressOnStack(Stack* stack, size_t  value);

uint8_t popByteFromStack(   Stack* stack);
int32_t popIntFromStack(    Stack* stack);
double  popFloatFromStack(  Stack* stack);
size_t  popAddressFromStack(Stack* stack);

void setByteOnStack(   Stack* stack, size_t address, uint8_t value);
void setIntOnStack(    Stack* stack, size_t address, int32_t value);
void setFloatOnStack(  Stack* stack, size_t address, double  value);
void setAddressOnStack(Stack* stack, size_t address, size_t  value);

uint8_t getByteFromStack(   const Stack* stack, size_t address);
int32_t getIntFromStack(    const Stack* stack, size_t address);
double  getFloatFromStack(  const Stack* stack, size_t address);
size_t  getAddressFromStack(const Stack* stack, size_t address);


#endif

