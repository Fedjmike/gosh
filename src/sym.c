#include "sym.h"

#include <assert.h>

#include "common.h"

static void symAddChild(sym* parent, sym* child) {
    assert(parent);
    assert(child);
    assert(!child->parent);

    vectorPush(&parent->children, child);
    child->parent = parent;
}

static sym* symCreate (symKind kind, const char* name, sym init) {
    sym* symbol = malloci(sizeof(sym), &init);
    symbol->kind = kind;
    symbol->name = strdup(name);
    return symbol;
}

static sym* symCreateParented (symKind kind, sym* parent, const char* name, sym init) {
    sym* symbol = symCreate(kind, name, init);
    symAddChild(parent, symbol);
    return symbol;
}

static void symDestroy (sym* symbol) {
    vectorFreeObjs(&symbol->children, (vectorDtor) symDestroy);
    free(symbol->name);
    free(symbol);
}

sym* symAdd (sym* parent, const char* name) {
    return symCreateParented(symNormal, parent, name, (sym) {});
}

sym* symAddScope (sym* parent) {
    return symCreateParented(symScope, parent, "<scope>", (sym) {
        .children = vectorInit(10, malloc)
    });
}

sym* symInit (void) {
    return symCreate(symScope, "<global scope>", (sym) {
        .children = vectorInit(50, malloc)
    });
}

void symEnd (sym* global) {
    symDestroy(global);
}

const char* symGetName (const sym* symbol) {
    return   !symbol ? "<no symbol attached>"
           : !symbol->name ? "<unnamed symbol>"
           : symbol->name;
}

sym* symLookup (const sym* scope, const char* name) {
    for (int i = scope->children.length; i; i--) {
        sym* symbol = vectorGet(scope->children, i-1);

        if (!strcmp(name, symbol->name))
            return symbol;
    }

    if (scope->parent)
        return symLookup(scope->parent, name);

    return 0;
}

bool symIsInside (const sym* look, const sym* scope) {
    if (look->parent == scope)
        return true;

    for_vector (sym* child, scope->children, {
        if (child->kind == symScope) {
            if (symIsInside(look, child))
                return true;
        }
    })

    return false;
}