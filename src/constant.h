#ifndef lala_constant_h
#define lala_constant_h


#include <stdint.h>
#include <stdio.h>


// ┌────────┐
// │ Macros │
// └────────┘

#define MAX_CONSTANTS 16


// ┌───────┐
// │ Types │
// └───────┘

typedef struct {
    uint8_t  length;
    const uint8_t* value;
} Constant;

typedef struct {
    Constant constants[MAX_CONSTANTS];
    uint8_t  count;
} Constants;


// ┌───────────────────────┐
// │ Function declarations │
// └───────────────────────┘

uint8_t addConstant(Constants* constants, uint8_t length, const uint8_t* value);
size_t getConstantSectionSize(const Constants* constants);

/* Serializes into the following presentation:
 *
 * 1. Constant table
 *     1 byte –– number of constants
 *     9 bytes for each constant –– constant length and constant value offset from the start
 *                                  of the constants section
 * 2. Constants values
 *     N bytes –– the values of constants
 */
void serializeConstants(FILE* out_file, const Constants* constants);
void deserializeConstants(
    const uint8_t* constants_section,
    size_t constants_section_size,
    Constants* constants
);


#endif

