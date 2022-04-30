#include "scope.h"


#include <assert.h>
#include <stdlib.h>


// ┌────────┐
// │ Macros │
// └────────┘

#define VALIDATE_SCOPE(scope)                           \
    (                                                   \
        scope &&                                        \
        scope != scope->parent &&                       \
        scope->variables_count < MAX_VARIABLES_IN_SCOPE \
    )

#define ASSERT_SCOPE(scope)                             \
    if (!VALIDATE_SCOPE(scope)) {                       \
        fprintf(stderr,                                 \
            "%s:%d, in %s:\nScope assertion failed.\n", \
            __FILENAME__,                               \
            __LINE__,                                   \
            __FUNCTION_NAME__                           \
        );                                              \
        fdumpScope(stderr, scope, 0);                   \
        exit(1);                                        \
    }


// ┌──────────────────────────┐
// │ Function implementations │
// └──────────────────────────┘

Scope* createScope(Scope* parent) {
    if (parent) {
        ASSERT_SCOPE(parent);
    }

    Scope* scope = calloc(1, sizeof(Scope));

    scope->parent = parent;
    scope->stack_top = parent ? parent->stack_top : 0;

    initHashMap(&scope->symbol_table);
    scope->variables_count = 0;

    ASSERT_SCOPE(scope);
    return scope;
}

Scope* deleteScope(Scope* scope) {
    ASSERT_SCOPE(scope);

    Scope* parent = scope->parent;
    
    freeHashMap(&scope->symbol_table);
    free(scope);

    return parent;
}

void dumpScope(const Scope* scope) {
    fdumpScope(stdout, scope, 0);
}

void fdumpScope(FILE* out, const Scope* scope, int padding) {
    assert(out);

#define printf(...)                                \
    {                                              \
        if (padding > 0) {                         \
            fprintf(out, "%*s", padding * 2, " "); \
        }                                          \
        fprintf(out, __VA_ARGS__);                 \
    }

    if (!scope) {
        fprintf(out, "Scope *(NULL)\n");
    } else {
        fprintf(out, "Scope *(%p) %s {\n",
            (const void*)scope,
            VALIDATE_SCOPE(scope) ? "VALID" : "INVALID"
        );

        if (scope->parent) {
            printf("  parent = *(%p)\n", (const void*)scope->parent);
        } else {
            printf("  parent = NULL\n");
        }
        printf("  stack_top = %zu\n", scope->stack_top);
        
        printf("  symbol_table = ");
        fdumpHashMap(out, &scope->symbol_table, padding + 1);

        printf("  variables[%zu/%d] = [\n", scope->variables_count, MAX_VARIABLES_IN_SCOPE);
        for (size_t i = 0; i < scope->variables_count; ++i) {
            printf("    [%zu] = {type = %s, address_on_stack = %zu}\n",
                i,
                valueTypeName(scope->variables[i].type),
                scope->variables[i].address_on_stack
            );
        }
        printf("  ]\n");

        printf("  variables_count = %zu\n", scope->variables_count);
        printf("}\n");
    }

#undef printf
}

bool declareVariableInScope(
    Scope* scope,
    const char* name,
    size_t name_length,
    ValueType type
) {
    ASSERT_SCOPE(scope);

    if (scope->variables_count == MAX_VARIABLES_IN_SCOPE) {
        return false;
    }

    if (hashMapContains(&scope->symbol_table, name, name_length)) {
        return false;
    }

    scope->variables[scope->variables_count].type = type;
    scope->variables[scope->variables_count].address_on_stack = scope->stack_top;
    scope->stack_top += valueTypeSize(type);

    assert(storeInHashMap(
        &scope->symbol_table,
        name,
        name_length,
        scope->variables_count
    ));
    scope->variables_count++;

    ASSERT_SCOPE(scope);
    return true;
}

bool accessVariableInScope(
    const Scope* scope,
    const char* name,
    size_t name_length,
    Variable* variable
) {
    ASSERT_SCOPE(scope);

    size_t variable_index;
    bool found = false;
    for (const Scope* i = scope; !found && i != NULL; i = i->parent) {
        found = getFromHashMap(&i->symbol_table, name, name_length, &variable_index);
        if (found) {
            *variable = i->variables[variable_index];
        }
    }

    ASSERT_SCOPE(scope);
    return found;
}

