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

typedef enum {
    VARDECL_SUCCESS,
    VARDECL_TOO_MANY_VARIABLES_IN_A_SCOPE,
    VARDECL_VARIABLE_REDECLARATION,
} VariableDeclarationResult;

typedef enum {
    GLOBAL_VARIABLE,
    LOCAL_VARIABLE,
} VariableKind;

typedef struct {
    VariableKind kind;
    ValueType* type;
    size_t address_on_stack;
} Variable;

struct Scope {
    struct Scope* parent;
    size_t stack_top;

    // name —> variables[] index
    HashMap symbol_table;
    Variable variables[MAX_VARIABLES_IN_SCOPE];
    size_t variables_count;

    // If return_type isn't NULL, it's a function's root scope.
    ValueType* return_type;
};
typedef struct Scope Scope;


// ┌───────────────────────┐
// │ Function declarations │
// └───────────────────────┘

Scope* createScope(Scope* parent);
Scope* createScopeInNewCallFrame(Scope* parent);
// Returns parent scope.
Scope* deleteScope(Scope* scope);

void dumpScope(const Scope* scope);
void fdumpScope(FILE* out, const Scope* scope, int padding);

VariableDeclarationResult declareVariableInScope(
    Scope* scope,
    const char* name,
    size_t name_lengh,
    ValueType* type
);

bool accessVariableInScope(
    const Scope* scope,
    const char* name,
    size_t name_length,
    Variable* variable
);

// Returns false if not in a function at the moment.
ValueType* getReturnType(const Scope* scope);


#endif

