#ifndef lala_vm_h
#define lala_vm_h


#include "parser.h"


// ┌────────┐
// │ Macros │
// └────────┘

#define EPSILON 1e-10


// ┌───────┐
// │ Types │
// └───────┘

typedef struct {
    uint8_t* source;
    uint8_t* ip;

    Stack stack;
} VM;


// ┌───────────────────────┐
// │ Function declarations │
// └───────────────────────┘

void initVM(VM* vm, uint8_t* source);
void freeVM(VM* vm);
void dumpVM(const VM* vm);
void fdumpVM(FILE* out, const VM* vm, int padding);

void interpret(VM* vm);


#endif

