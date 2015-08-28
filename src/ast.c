#include "ast.h"

#include <assert.h>
#include <stdlib.h>
#include <vector.h>

#include "common.h"

static ast* astCreate (astKind kind, ast init) {
    assert(kind != astKindNo);

    ast* node = malloci(sizeof(*node), &init);
    node->kind = kind;
    return node;
}

void astDestroy (ast* node) {
    if (node->l)
        astDestroy(node->l);

    if (node->r)
        astDestroy(node->r);

    vectorFreeObjs(&node->children, (vectorDtor) astDestroy);

    if (   node->kind == astStrLit || node->kind == astFileLit
        || node->kind == astGlobLit)
        free(node->literal.str);

    if (node->kind == astFnLit && node->captured) {
        vectorFree(node->captured);
        /*Stored indirectly*/
        free(node->captured);
    }

    free(node);
}

ast* astCreateBOP (ast* l, ast* r, opKind op) {
    return astCreate(astBOP, (ast) {
        .l = l,
        .r = r,
        .op = op
    });
}

ast* astCreateFnApp (vector(ast*) args, ast* fn) {
    return astCreate(astFnApp, (ast) {
        .r = fn,
        .children = args
    });
}

ast* astCreateSymbol (sym* symbol, bool captured) {
    return astCreate(astSymbol, (ast) {
        .symbol = symbol,
        .flags = captured ? astCaptured : astNoFlags
    });
}

ast* astCreateUnitLit (void) {
    return astCreate(astUnitLit, (ast) {});
}

ast* astCreateIntLit (int64_t integer) {
    return astCreate(astIntLit, (ast) {
        .literal.integer = integer,
    });
}

ast* astCreateBoolLit (bool truth) {
    return astCreate(astBoolLit, (ast) {
        .literal.truth = truth,
    });
}

ast* astCreateStrLit (const char* str) {
    return astCreate(astStrLit, (ast) {
        .literal.str = strdup(str),
    });
}

ast* astCreateFileLit (const char* str) {
    return astCreate(astFileLit, (ast) {
        .literal.str = strdup(str),
    });
}

ast* astCreateGlobLit (const char* str) {
    return astCreate(astGlobLit, (ast) {
        .literal.str = strdup(str),
    });
}

ast* astCreateListLit (vector(ast*) elements) {
    return astCreate(astListLit, (ast) {
        .children = elements,
    });
}

ast* astCreateTupleLit (vector(ast*) elements) {
    return astCreate(astTupleLit, (ast) {
        .children = elements,
    });
}

ast* astCreateFnLit (vector(ast*) args, ast* expr, vector(sym*) captured) {
    return astCreate(astFnLit, (ast) {
        .children = args, .r = expr,
        .captured = malloci(sizeof(vector), &captured)
    });
}

ast* astCreateLet (sym* symbol, ast* init) {
    return astCreate(astLet, (ast) {
        .symbol = symbol, .r = init
    });
}

ast* astCreateInvalid (void) {
    return astCreate(astInvalid, (ast) {});
}

/*==== ====*/

const char* opKindGetStr (opKind kind) {
    switch (kind) {
    case opPipe: return "|";
    case opWrite: return "|>";
    case opLogicalAnd: return "&&";
    case opLogicalOr: return "||";
    case opEqual: return "==";
    case opNotEqual: return "!=";
    case opLess: return "<";
    case opLessEqual: return "<=";
    case opGreater: return ">";
    case opGreaterEqual: return ">=";
    case opAdd: return "+";
    case opSubtract: return "-";
    case opConcat: return "++";
    case opMultiply: return "*";
    case opDivide: return "/";
    case opModulo: return "%";
    case opNull: return "<null op kind>";
    }

    return "<unhandled op kind>";
}

const char* astKindGetStr (astKind kind) {
    switch (kind) {
    case astBOP: return "BOP";
    case astFnApp: return "FnApp";
    case astSymbol: return "Symbol";
    case astUnitLit: return "UnitLit";
    case astIntLit: return "IntLit";
    case astBoolLit: return "BoolLit";
    case astStrLit: return "StrLit";
    case astFileLit: return "FileLit";
    case astGlobLit: return "GlobLit";
    case astListLit: return "ListLit";
    case astTupleLit: return "TupleLit";
    case astFnLit: return "FnLit";
    case astLet: return "Let";
    case astInvalid: return "Invalid";
    case astKindNo: return "<KindNo; not real>";
    }

    return "<unhandled AST kind>";
}

const char* astFlagGetStr (astFlags flag) {
    switch (flag) {
    case astNoFlags: return "NoFlags";
    case astCaptured: return "Captured";
    case astUnixInvocation: return "UnixInvocation";
    case astListApplication: return "ListApplication";
    }

    return "<unhandled AST flag>";
}