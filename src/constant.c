#include "constant.h"


#include <assert.h>


// ┌──────────────────┐
// │ Static functions │
// └──────────────────┘

static size_t getConstantTableSize(const Constants* constants) {
    assert(constants);
    return sizeof(uint8_t) + constants->count * (sizeof(uint8_t) + sizeof(size_t));
}

static size_t getConstantValuesSize(const Constants* constants) {
    assert(constants);

    size_t size = 0;
    for (uint8_t i = 0; i < constants->count; ++i) {
        size += constants->constants[i].length;
    }
    return size;
}


// ┌──────────────────────────┐
// │ Function implementations │
// └──────────────────────────┘

uint8_t addConstant(Constants* constants, uint8_t length, const uint8_t* value) {
    assert(constants);
    assert(constants->count < MAX_CONSTANTS);
    assert(value);

    constants->constants[constants->count].length = length;
    constants->constants[constants->count].value = value;
    constants->count += 1;

    return constants->count - 1;
}

size_t getConstantSectionSize(const Constants* constants) {
    return getConstantTableSize(constants) + getConstantValuesSize(constants);
}

void serializeConstants(FILE* out_file, const Constants* constants) {
    assert(constants);
    assert(out_file);

    size_t next_constant_value_offset = getConstantTableSize(constants);

    fwrite(&constants->count, 1, sizeof(uint8_t), out_file);
    for (uint8_t i = 0; i < constants->count; ++i) {
        fwrite(&constants->constants[i].length, 1, sizeof(uint8_t), out_file);
        fwrite(&next_constant_value_offset, 1, sizeof(size_t), out_file);
        next_constant_value_offset += constants->constants[i].length;
    }

    for (uint8_t i = 0; i < constants->count; ++i) {
        fwrite(constants->constants[i].value, 1, constants->constants[i].length, out_file);
    }
}

void deserializeConstants(
    const uint8_t* constants_section,
    size_t constants_section_size,
    Constants* constants
) {
    assert(constants);
    assert(constants_section);

    constants->count = *constants_section;
    const uint8_t* constant_ptr = constants_section + sizeof(uint8_t);

    for (uint8_t i = 0; i < constants->count; ++i) {
        constants->constants[i].length = *constant_ptr;
        constant_ptr += sizeof(uint8_t);

        const size_t value_offset = *(const size_t*)constant_ptr;
        assert(value_offset + constants->constants[i].length <= constants_section_size);

        constants->constants[i].value = constants_section + value_offset;
        constant_ptr += sizeof(size_t);
    }
}

