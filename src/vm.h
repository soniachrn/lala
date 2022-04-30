#ifndef lala_vm_h
#define lala_vm_h


#include "constant.h"
#include "heap.h"
#include "op_code.h"
#include "stack.h"


// ┌────────┐
// │ Macros │
// └────────┘

#define EPSILON 1e-10


// ┌───────┐
// │ Types │
// └───────┘

typedef struct {
    size_t   source_size;
    uint8_t* source;
    uint8_t* ip;

    Constants* constants;
    Stack stack;
    Heap heap;
} VM;


// ┌───────────────────────┐
// │ Function declarations │
// └───────────────────────┘

void initVM(VM* vm, uint8_t* source, size_t source_size, Constants* constants);
void freeVM(VM* vm);
void dumpVM(const VM* vm);
void fdumpVM(FILE* out, const VM* vm, int padding);

void interpret(VM* vm);


#endif

