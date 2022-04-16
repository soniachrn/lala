#ifndef lala_scope_h
#define lala_scope_h


#include "hashmap.h"
#include "value_type.h"


// ┌────────┐
// │ Macros │
// └────────┘

#define MAX_VARIABLES_IN_SCOPE 128


// ┌───────┐
// │ Types │
// └───────┘

typedef struct {
    ValueType type;
    size_t address_on_stack;
} Variable;

struct Scope {
    struct Scope* parent;
    size_t stack_top;

    // name —> variables[] index
    HashMap symbol_table;
    Variable variables[MAX_VARIABLES_IN_SCOPE];
    size_t variables_count;
};
typedef struct Scope Scope;


// ┌───────────────────────┐
// │ Function declarations │
// └───────────────────────┘

Scope* createScope(Scope* parent);
// Returns parent scope.
Scope* deleteScope(Scope* scope);

void dumpScope(const Scope* scope);
void fdumpScope(FILE* out, const Scope* scope, int padding);

bool declareVariableInScope(
    Scope* scope,
    const char* name,
    size_t name_lengh,
    ValueType type
);

bool accessVariableInScope(
    const Scope* scope,
    const char* name,
    size_t name_length,
    Variable* variable
);


#endif

