#include "hashmap.h"


#include <assert.h>
#include <stdlib.h>


// ┌────────┐
// │ Macros │
// └────────┘

#define VALIDATE_HASH_MAP(map)     \
    (                              \
        map &&                     \
        map->entries &&            \
        map->capacity > map->count \
    )

#define ASSERT_HASH_MAP(map)                              \
    if (!VALIDATE_HASH_MAP(map)) {                        \
        fprintf(stderr,                                   \
            "%s:%d, in %s:\nHashMap assertion failed.\n", \
            __FILENAME__,                                 \
            __LINE__,                                     \
            __FUNCTION_NAME__                             \
        );                                                \
        fdumpHashMap(stderr, map, 0);                     \
        exit(1);                                          \
    }

#define IS_TOMBSTONE(entry) (entry->key == NULL && entry->key_length == 1)


// ┌──────────────────────────────┐
// │ Static function declarations │
// └──────────────────────────────┘

static bool storeInHashMapKnownHash(
    HashMap* map,
    const char* string,
    size_t length,
    uint32_t hash,
    size_t value
);

static HashMapEntry* findEntry(
    const HashMap* map,
    const char* string,
    size_t length,
    uint32_t hash
);

static void growIfNeeded(HashMap* map);
static void shrinkIfNeeded(HashMap* map);
static void resizeHashMap(HashMap* map, size_t new_capacity);


// ┌──────────────────────────┐
// │ Function implementations │
// └──────────────────────────┘

uint32_t calculateHash(const char* string, size_t length) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < length; ++i) {
        hash ^= (uint8_t)string[i];
        hash *= 16777619;
    }
    return hash;
}

void initHashMap(HashMap* map) {
    assert(map);

    map->entries = calloc(INITIAL_HASH_MAP_SIZE, sizeof(HashMapEntry));
    map->capacity = INITIAL_HASH_MAP_SIZE;
    map->count = 0;

    ASSERT_HASH_MAP(map);
}

void freeHashMap(HashMap* map) {
    ASSERT_HASH_MAP(map);

    free(map->entries);
    map->capacity = 0;
    map->count = 0;
}

void dumpHashMap(const HashMap* map) {
    fdumpHashMap(stdout, map, 0);
}

void fdumpHashMap(FILE* out, const HashMap* map, int padding) {
    (void)out;
    (void)map;
    (void)padding;
    fprintf(out, "\n");
}

bool storeInHashMap(HashMap* map, const char* key, size_t key_length, size_t value) {
    ASSERT_HASH_MAP(map);

    uint32_t hash = calculateHash(key, key_length);
    bool is_new_key = storeInHashMapKnownHash(map, key, key_length, hash, value);

    ASSERT_HASH_MAP(map);
    return is_new_key;
}

bool getFromHashMap(
    const HashMap* map,
    const char* key,
    size_t key_length,
    size_t* value
) {
    ASSERT_HASH_MAP(map);

    if (map->count == 0)
        return false;

    uint32_t hash = calculateHash(key, key_length);
    HashMapEntry* entry = findEntry(map, key, key_length, hash);
    if (entry->key == NULL) {
        return false;
    }

    *value = entry->value;

    ASSERT_HASH_MAP(map);
    return true;
}

bool removeFromHashMap(HashMap* map, const char* key, size_t key_length) {
    ASSERT_HASH_MAP(map);

    uint32_t hash = calculateHash(key, key_length);
    HashMapEntry* entry = findEntry(map, key, key_length, hash);
    if (entry->key == NULL) {
        return false;
    }

    // (key == NULL && key_length == 1) represents a tombstone.
    // This condition may be true only after a removal of an element, 
    // because all entries are initialized with 0-s when the memory
    // is allocated with calloc.
    entry->key = NULL;
    entry->key_length = 1;
    entry->hash = 0;
    entry->value = 0;

    shrinkIfNeeded(map);

    ASSERT_HASH_MAP(map);
    return true;
}

bool hashMapContains(const HashMap* map, const char* key, size_t key_length) {
    ASSERT_HASH_MAP(map);

    uint32_t hash = calculateHash(key, key_length);
    HashMapEntry* entry = findEntry(map, key, key_length, hash);

    return entry->key != NULL;
}


// ┌─────────────────────────────────┐
// │ Static function implementations │
// └─────────────────────────────────┘

static bool storeInHashMapKnownHash(
    HashMap* map,
    const char* key,
    size_t key_length,
    uint32_t hash,
    size_t value
) {
    ASSERT_HASH_MAP(map);

    HashMapEntry* entry = findEntry(map, key, key_length, hash);

    bool is_new_key = entry->key == NULL;
    if (is_new_key && !IS_TOMBSTONE(entry)) {
        map->count += 1;
    }

    entry->key = key;
    entry->key_length = key_length;
    entry->hash = hash;
    entry->value = value;

    growIfNeeded(map);

    ASSERT_HASH_MAP(map);
    return is_new_key;
}

static HashMapEntry* findEntry(
    const HashMap* map,
    const char* key,
    size_t length,
    uint32_t hash
) {
    ASSERT_HASH_MAP(map);

    HashMapEntry* tombstone = NULL;

    for (size_t i = hash % map->capacity; ; i = (i + 1) % map->capacity) {
        HashMapEntry* entry = &map->entries[i];
        
        // Either a tombstone or an empty entry.
        if (entry->key == NULL) {
            // Tombstone
            if (IS_TOMBSTONE(entry)) {
                tombstone = tombstone ? tombstone : entry;
            }
            // Empty entry
            else {
                return tombstone ? tombstone : entry;
            }
        }
        
        // The entry corresponding to the key.
        else if (
            entry->hash == hash &&
            entry->key_length == length &&
            memcmp(entry->key, key, length) == 0
        ) {
            return entry;
        }
    }
}

void growIfNeeded(HashMap* map) {
    ASSERT_HASH_MAP(map);

    if (map->count > map->capacity * HASH_MAP_MAX_LOAD_FACTOR) {
        resizeHashMap(map, map->capacity * 2);
    }

    ASSERT_HASH_MAP(map);
}

void shrinkIfNeeded(HashMap* map) {
    ASSERT_HASH_MAP(map);

    if (map->capacity > INITIAL_HASH_MAP_SIZE && 
        map->count < map->capacity * HASH_MAP_MIN_LOAD_FACTOR
    ) {
        resizeHashMap(map, map->capacity / 2);
    }

    ASSERT_HASH_MAP(map);
}

void resizeHashMap(HashMap* map, size_t new_capacity) {
    ASSERT_HASH_MAP(map);

    HashMapEntry* old_entries = map->entries;
    size_t old_capacity = map->capacity;

    map->entries = calloc(new_capacity, sizeof(HashMapEntry));
    if (!map->entries) {
        // TODO: error
        assert(false);
    }
    map->capacity = new_capacity;
    map->count = 0;

    for (size_t i = 0; i < old_capacity; ++i) {
        if (old_entries[i].key != NULL) {
            storeInHashMapKnownHash(
                map,
                old_entries[i].key,
                old_entries[i].key_length,
                old_entries[i].hash,
                old_entries[i].value
            );
        }
    }

    free(old_entries);

    ASSERT_HASH_MAP(map);
}


#undef IS_TOMBSTONE

#undef ASSERT_HASH_MAP
#undef VALIDATE_HASH_MAP

