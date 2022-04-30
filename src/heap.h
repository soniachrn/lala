#ifndef lala_heap_h
#define lala_heap_h


#include <stdint.h>
#include <stdio.h>


struct Object {
    struct Object* reference_rule;
    size_t   size;
    uint8_t* value;
    struct Object*  next;
};
typedef struct Object Object;

typedef struct Object ReferenceRule;

typedef struct {
    Object* first;
} Heap;


extern ReferenceRule REFERENCE_RULE_PLAIN;
extern Object OBJECT_STRING_TRUE;
extern Object OBJECT_STRING_FALSE;


void initHeap(Heap* heap);
void freeHeap(Heap* heap);
void dumpHeap(Heap* heap);
void fdumpHeap(FILE* out, Heap* heap, int padding);

Object* allocateEmptyObject(
    Heap* heap,
    ReferenceRule* reference_rule,
    size_t size
);

Object* allocateObjectFromValue(
    Heap* heap,
    ReferenceRule* reference_rule,
    size_t size,
    const uint8_t* value_source
);

void deallocateObject(Heap* heap, Object* object);


#endif

