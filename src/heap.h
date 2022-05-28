#ifndef lala_heap_h
#define lala_heap_h


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "stack.h"


// ┌────────┐
// │ Macros │
// └────────┘

// #define DEBUG_HEAP
// #define STRESS_GC

#ifdef STRESS_GC
    #define GC_INITIAL_THRESHOLD            0
    #define GC_THRESHOLD_HEAP_GROWTH_FACTOR 0
#else
    #define GC_INITIAL_THRESHOLD            (1024 * 1024)
    #define GC_THRESHOLD_HEAP_GROWTH_FACTOR 2
#endif


// ┌───────┐
// │ Types │
// └───────┘

typedef enum {
    REFERENCE_RULE_PLAIN,
    REFERENCE_RULE_REF_ARRAY,
    REFERENCE_RULE_CUSTOM,
} ReferenceRule;

struct Object;
typedef struct Object Object;

struct Object {
    ReferenceRule reference_rule;
    Object* custom_reference_rule;
    size_t size;
    uint8_t* value;
    Object* next;
    bool marked;
};

typedef struct {
    Object* first;
    size_t size;
    size_t next_gc;
} Heap;


// ┌────────────────────────┐
// │ Constants declarations │
// └────────────────────────┘

extern Object OBJECT_STRING_TRUE;
extern Object OBJECT_STRING_FALSE;


// ┌───────────────────────┐
// │ Function declarations │
// └───────────────────────┘

const char* referenceRuleName(ReferenceRule reference_rule);

void initHeap(Heap* heap);
void freeHeap(Heap* heap);
void dumpHeap(const Heap* heap);
void fdumpHeap(FILE* out, const Heap* heap, int padding);

void dumpObject(const Object* object);
void fdumpObject(FILE* out, const Object* object, int padding);

Object* allocateEmptyObject(
    Heap* heap,
    const Stack* stack,
    const Stack* stack_references_positions,
    ReferenceRule reference_rule,
    Object* custom_reference_rule,
    size_t size
);

Object* allocateObjectFromValue(
    Heap* heap,
    const Stack* stack,
    const Stack* stack_references_positions,
    ReferenceRule reference_rule,
    Object* custom_reference_rule,
    size_t size,
    const uint8_t* value_source
);

/* Prevents an object from being collected on the next gc.
 * Is useful when an object that's already popped from stack
 * (this isn't marked as root) isn't used yet. Look at 
 * OP_CONCATENATE in VM for example.
 * */
void dontCollectObjectOnNextGC(Object* object);


#endif

