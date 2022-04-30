#ifndef lala_hashmap_h
#define lala_hashmap_h


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "debug.h"


// ┌────────┐
// │ Macros │
// └────────┘

#define INITIAL_HASH_MAP_SIZE 8
#define HASH_MAP_MAX_LOAD_FACTOR     0.5
#define HASH_MAP_REALLOC_LOAD_FACTOR 0.25
#define HASH_MAP_MIN_LOAD_FACTOR     0.125


// ┌───────┐
// │ Types │
// └───────┘

typedef struct {
    const char* key;
    size_t key_length;
    uint32_t hash;
    size_t value;
} HashMapEntry;

typedef struct {
    HashMapEntry* entries;
    size_t capacity;
    size_t count;
} HashMap;


// ┌───────────────────────┐
// │ Function declarations │
// └───────────────────────┘

uint32_t calculateHash(const char* string, size_t length);

void initHashMap(HashMap* map);
void freeHashMap(HashMap* map);
void dumpHashMap(const HashMap* map);
void fdumpHashMap(FILE* out, const HashMap* map, int padding);

bool storeInHashMap(
    HashMap* map,
    const char* key,
    size_t key_length,
    size_t value
);
bool getFromHashMap(
    const HashMap* map,
    const char* key,
    size_t key_length,
    size_t* value
);
bool removeFromHashMap(
    HashMap* map,
    const char* key,
    size_t key_length
);
bool hashMapContains(
    const HashMap* map,
    const char* key,
    size_t key_length
);


#endif

