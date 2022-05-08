#include "heap.h"


#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"


// ┌────────┐
// │ Macros │
// └────────┘

#define VALIDATE_OBJECT(object)                                   \
    (                                                             \
        object &&                                                 \
        (                                                         \
            object->reference_rule == REFERENCE_RULE_PLAIN     || \
            object->reference_rule == REFERENCE_RULE_REF_ARRAY || \
            object->reference_rule == REFERENCE_RULE_CUSTOM       \
        ) &&                                                      \
        (                                                         \
            object->reference_rule != REFERENCE_RULE_CUSTOM ||    \
            object->custom_reference_rule                         \
        ) &&                                                      \
        (                                                         \
            object->reference_rule == REFERENCE_RULE_CUSTOM ||    \
            !object->custom_reference_rule                        \
        ) &&                                                      \
        object->size > 0 &&                                       \
        object->value                                             \
    )

#define ASSERT_OBJECT(object)                            \
    if (!VALIDATE_OBJECT(object)) {                      \
        fprintf(stderr,                                  \
            "%s:%d, in %s:\nObject assertion failed.\n", \
            __FILENAME__,                                \
            __LINE__,                                    \
            __FUNCTION_NAME__                            \
        );                                               \
        fdumpObject(stderr, object, 0);                  \
        exit(1);                                         \
    }


// ┌───────────────────────┐
// │ Constants definitions │
// └───────────────────────┘

Object OBJECT_STRING_TRUE  = { REFERENCE_RULE_PLAIN, NULL, 4, (uint8_t*)"true",  NULL, true };
Object OBJECT_STRING_FALSE = { REFERENCE_RULE_PLAIN, NULL, 5, (uint8_t*)"false", NULL, true };


// ┌──────────────────────────────┐
// │ Static function declarations │
// └──────────────────────────────┘

static void collectGarbage(
    Heap* heap,
    const Stack* stack,
    const Stack* stack_references_positions
);

static void markObject(Object* object);


// ┌──────────────────────────┐
// │ Function implementations │
// └──────────────────────────┘

const char* referenceRuleName(ReferenceRule reference_rule) {
    switch (reference_rule) {
        case REFERENCE_RULE_PLAIN:     return "plain";
        case REFERENCE_RULE_REF_ARRAY: return "ref array";
        case REFERENCE_RULE_CUSTOM:    return "custom";
        default:                       return "INVALID REFERENCE RULE";
    }
}

void initHeap(Heap* heap) {
    assert(heap);

    heap->first   = NULL;
    heap->size    = 0;
    heap->next_gc = GC_INITIAL_THRESHOLD;
}

void freeHeap(Heap* heap) {
    assert(heap);

    while (heap->first != NULL) {
        Object* next = heap->first->next;
        deallocateObject(heap, heap->first);
        heap->first = next;
    }
    heap->size    = 0;
    heap->next_gc = GC_INITIAL_THRESHOLD;
}

void dumpHeap(const Heap* heap) {
    fdumpHeap(stdout, heap, 0);
}

void fdumpHeap(FILE* out, const Heap* heap, int padding) {
    assert(out);
    assert(heap);

#define printf(...)                                \
    {                                              \
        if (padding > 0) {                         \
            fprintf(out, "%*s", padding * 2, " "); \
        }                                          \
        fprintf(out, __VA_ARGS__);                 \
    }

    if (!heap) {
        fprintf(out, "Heap *(NULL)\n");
    } else {
        fprintf(out, "Heap *(%p) {\n", (const void*)heap);

        printf("  size = %ld\n", heap->size);
        printf("  next_gc = %ld\n", heap->next_gc);
        printf("  objects = [\n");
        for (Object* i = heap->first; i != NULL; i = i->next) {
            printf("    ");
            fdumpObject(out, i, padding + 2);
        }
        printf("  ]\n");
        printf("}\n");
    }

#undef printf
}

void dumpObject(const Object* object) {
    fdumpObject(stdout, object, 0);
}

void fdumpObject(FILE* out, const Object* object, int padding) {
    assert(out);

#define printf(...)                                \
    {                                              \
        if (padding > 0) {                         \
            fprintf(out, "%*s", padding * 2, " "); \
        }                                          \
        fprintf(out, __VA_ARGS__);                 \
    }

    if (!object) {
        fprintf(out, "Object *(NULL)\n");
    } else {
        fprintf(out, "Object *(%p) %s {\n",
            (const void*)object,
            VALIDATE_OBJECT(object) ? "VALID" : "INVALID"
        );

        printf("  reference_rule = %s\n", referenceRuleName(object->reference_rule));
        printf("  custom_reference_rule = ");
        if (object->custom_reference_rule) {
            fprintf(out, "*(%p)\n", (void*)object->custom_reference_rule);
            // fdumpObject(out, object->custom_reference_rule, padding + 1);
        } else {
            fprintf(out, "*(NULL)\n");
        }
        printf("  size = %ld\n", object->size);
        printf("  value = ");
        if (object->value) {
            fprintf(out, "[\n");
            for (size_t i = 0; i < object->size;) {
                printf("    ");
                for (size_t j = 0; j < 8 && i < object->size; ++j) {
                    fprintf(out, "%02X ", object->value[i]);
                    ++i;
                }
                fprintf(out, "\n");
            }
            fprintf(out, "  ]\n");
        } else {
            fprintf(out, "*(NULL)\n");
        }
        printf("  next = ");
        if (object->next) {
            fprintf(out, "*(%p)\n", (void*)object->next);
        } else {
            fprintf(out, "*(NULL)\n");
        }
        printf("  marked = %s\n", object->marked ? "true" : "false");
        printf("}\n");
    }

#undef printf
}

Object* allocateEmptyObject(
    Heap* heap,
    const Stack* stack,
    const Stack* stack_references_positions,
    ReferenceRule reference_rule,
    Object* custom_reference_rule,
    size_t size
) {
    assert(heap);
    assert(stack);
    assert(stack_references_positions);

    if (heap->size >= heap->next_gc) {
        collectGarbage(heap, stack, stack_references_positions);
    }

    Object* object = calloc(sizeof(Object), 1);
    // TODO: check that object has been allocated
    
    printf("allocate %p\n", (void*)object);

    object->reference_rule = reference_rule;
    object->custom_reference_rule = custom_reference_rule;
    object->size = size;
    object->value = malloc(size);
    object->next = heap->first;
    object->marked = false;

    heap->first = object;
    heap->size += sizeof(Object) + size;

    ASSERT_OBJECT(object);
    return object;
}

Object* allocateObjectFromValue(
    Heap* heap,
    const Stack* stack,
    const Stack* stack_references_positions,
    ReferenceRule reference_rule,
    Object* custom_reference_rule,
    size_t size,
    const uint8_t* value_source
) {
    assert(heap);
    assert(stack);
    assert(stack_references_positions);
    assert(value_source);

    Object* object = allocateEmptyObject(
        heap,
        stack,
        stack_references_positions,
        reference_rule,
        custom_reference_rule,
        size
    );
    memcpy(object->value, value_source, size);

    ASSERT_OBJECT(object);
    return object;
}

void deallocateObject(Heap* heap, Object* object) {
    assert(heap);
    ASSERT_OBJECT(object);

    printf("deallocate %p\n", (void*)object);

    heap->size -= sizeof(Object) + object->size;
    free(object->value);
    free(object);
}


// ┌─────────────────────────────────┐
// │ Static function implementations │
// └─────────────────────────────────┘

static void collectGarbage(
    Heap* heap,
    const Stack* stack,
    const Stack* stack_references_positions
) {
    assert(heap);
    assert(stack);
    assert(stack_references_positions);

    printf("\nGC\n");
    printf("heap size before start: %ld\n", heap->size);

    // Mark.
    for (
        size_t* reference_position = (size_t*)stack_references_positions->stack;
        (uint8_t*)reference_position < stack_references_positions->stack_top;
        ++reference_position
    ) {
        markObject((Object*)getAddressFromStack(stack, *reference_position));
    }

    // Sweep.
    Object* last_reachable_object;
    for (Object* object = heap->first; object != NULL;) {
        Object* next = object->next;

        // Unmark an object for the future garbage collections.
        if (object->marked) {
            object->marked = false;
            last_reachable_object = object;
        }

        // Delete the unreachable object.
        else {
            if (object == heap->first) {
                heap->first = object->next;
            } else {
                assert(last_reachable_object);
                last_reachable_object->next = object->next;
            }
            deallocateObject(heap, object);
        }

        object = next;
    }

    printf("heap size after end: %ld\n\n", heap->size);

    // Calculate next gc threshold.
    heap->next_gc = heap->size * GC_THRESHOLD_HEAP_GROWTH_FACTOR;
}

static void markObject(Object* object) {
    ASSERT_OBJECT(object);

    if (object->marked) {
        return;
    }

    object->marked = true;
    switch (object->reference_rule) {
        case REFERENCE_RULE_PLAIN:
            break;
        
        case REFERENCE_RULE_REF_ARRAY:
            for (
                Object* array_item_object = (Object*)object->value;
                (uint8_t*)array_item_object < object->value + object->size;
                ++array_item_object
            ) {
                markObject(array_item_object);
            }
            break;

        case REFERENCE_RULE_CUSTOM: {
            Object* custom_rule = object->custom_reference_rule;
            markObject(custom_rule);
            for (
                size_t* reference_offset = (size_t*)custom_rule->value;
                (uint8_t*)reference_offset < custom_rule->value + custom_rule->size;
                ++reference_offset
            ) {
                assert(*reference_offset + sizeof(size_t) <= object->size);
                markObject((Object*)*(size_t*)(object->value + *reference_offset));
            }
            break;
        }
        
        default:
            assert(false);
    }
}

