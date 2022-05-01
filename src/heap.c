#include "heap.h"


#include <assert.h>
#include <stdlib.h>
#include <string.h>


// ┌───────────────────────┐
// │ Constants definitions │
// └───────────────────────┘

Object OBJECT_STRING_TRUE  = { REFERENCE_RULE_PLAIN, NULL, 4, (uint8_t*)"true",  NULL };
Object OBJECT_STRING_FALSE = { REFERENCE_RULE_PLAIN, NULL, 5, (uint8_t*)"false", NULL };


// ┌──────────────────────────┐
// │ Function implementations │
// └──────────────────────────┘

void initHeap(Heap* heap) {
    assert(heap);

    heap->first = NULL;
}

void freeHeap(Heap* heap) {
    assert(heap);

    while (heap->first != NULL) {
        Object* next = heap->first->next;
        deallocateObject(heap, heap->first);
        heap->first = next;
    }
}

void dumpHeap(Heap* heap) {
    fdumpHeap(stdout, heap, 0);
}

void fdumpHeap(FILE* out, Heap* heap, int padding) {
    assert(out);
    assert(heap);

    (void)out;
    (void)heap;
    (void)padding;
}

Object* allocateEmptyObject(
    Heap* heap,
    ReferenceRule reference_rule,
    Object* custom_reference_rule,
    size_t size
) {
    assert(heap);

    Object* object = calloc(sizeof(Object), 1);
    // TODO: check that object has been allocated
    
    object->reference_rule = reference_rule;
    object->custom_reference_rule = custom_reference_rule;
    object->size = size;
    object->value = malloc(size);
    object->next = heap->first;

    heap->first = object;

    return object;
}

Object* allocateObjectFromValue(
    Heap* heap,
    ReferenceRule reference_rule,
    Object* custom_reference_rule,
    size_t size,
    const uint8_t* value_source
) {
    assert(heap);
    assert(value_source);

    Object* object = allocateEmptyObject(heap, reference_rule, custom_reference_rule, size);
    memcpy(object->value, value_source, size);

    return object;
}

void deallocateObject(Heap* heap, Object* object) {
    (void)heap;
    free(object->value);
    free(object);
}

