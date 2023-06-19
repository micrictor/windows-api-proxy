#include <windows.h>
#include <signal.h>

#include <map>
#include <string>
#include <vector>
#include <variant>

#include "invoker_ioeo.h"
#include "thunk.h"

__attribute__((naked)) BOOL LoadLibraryIOEOCallback(
    PINIT_ONCE  InitOnce,
    argVector_t *args,
    PVOID       *Context) {
    unsigned long long retPtr;
    asm inline("movq (%%rsp), %0;"
        : "=r"(retPtr)
        : /* no input */);
    JmpThunk(args->size() - 1, args->data(), retPtr);
}

bool
IOEOExectutor::Invoke() {
    if (arguments.size() > MAX_ARGS) {
        return false;
    }

    INIT_ONCE initOnce = INIT_ONCE_STATIC_INIT;

    InitOnceExecuteOnce(&initOnce, (PINIT_ONCE_FN) LoadLibraryIOEOCallback, &arguments, NULL);
    return true;
}