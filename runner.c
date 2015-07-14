#include "runner.h"

#include <assert.h>
#include <gc/gc.h>
#include <sys/stat.h>

#include "sym.h"
#include "ast.h"
#include "value.h"

static value* impl_size__ (value* file) {
    const char* filename = valueGetFilename(file);

    if (!filename)
        return valueCreateInvalid();

    struct stat st;
    bool fail = stat(filename, &st);

    if (fail)
        return valueCreateInvalid();

    return valueCreateInt(st.st_size);
}

static value* runPipeApp (envCtx* env, const ast* node) {
    value *fn = run(env, node->l),
          *arg = run(env, node->r);

    /*Implicit map*/
    if (node->listApp) {
        valueIter iter;
        /*Fails if the arg isn't an iterable value*/
        assert(valueGetIterator(arg, &iter));

        vector(value*) results = vectorInit(valueGuessIterSize(iter), GC_malloc);

        /*Apply it to each element*/
        for (value* element; (element = valueIterRead(&iter));)
            vectorPush(&results, valueCall(fn, element));

        return valueCreateVector(results);

    } else
        return valueCall(fn, arg);
}

static value* runFnApp (envCtx* env, const ast* node) {
    value* result = run(env, node->l);

    for (int i = 0; i < node->children.length; i++) {
        ast* argNode = vectorGet(node->children, i);
        value* arg = run(env, argNode);

        result = valueCall(result, arg);
    }

    return result;
}

static value* runStrLit (envCtx* env, const ast* node) {
    (void) env;

    return valueCreateFile(node->literal.str);
}

static value* runListLit (envCtx* env, const ast* node) {
    vector(value*) result = vectorInit(node->children.length, GC_malloc);

    for (int i = 0; i < node->children.length; i++) {
        ast* element = vectorGet(node->children, i);
        vectorPush(&result, run(env, element));
    }

    return valueCreateVector(result);
}

static value* runSymbolLit (envCtx* env, const ast* node) {
    (void) env;

    if (!strcmp(node->literal.symbol->name, "size"))
        return valueCreateFn(impl_size__);

    else
        return valueCreateInvalid();
}

value* run (envCtx* env, const ast* node) {
    typedef value* (*handler_t)(envCtx*, const ast*);

    static handler_t table[astKindNo] = {
        [astPipeApp] = runPipeApp,
        [astFnApp] = runFnApp,
        [astStrLit] = runStrLit,
        [astListLit] = runListLit,
        [astSymbolLit] = runSymbolLit
    };

    handler_t handler = table[node->kind];

    if (handler)
        return handler(env, node);

    else {
        errprintf("Unhandled AST kind, %s\n", astKindGetStr(node->kind));
        return valueCreateInvalid();
    }
}
