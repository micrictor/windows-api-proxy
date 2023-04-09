#ifndef LOADLIBRARY_H
#define LOADLIBRARY_H

#include <windows.h>

__attribute__((naked)) PTP_WORK_CALLBACK LoadLibraryThreadpoolCallback(
    PTP_CALLBACK_INSTANCE       Instance,
    UINT_PTR                    *args[],
    PTP_WORK                    Work) {
    JmpThunk(1, *args);
}

__attribute__((naked)) PFLS_CALLBACK_FUNCTION LoadLibraryFlsCallback(
    UINT_PTR *args[]) {
    JmpThunk(1, *args);
}

void _InvokeLoadLibraryTp(LPCSTR lpLibFileName) {
    PTP_WORK work = NULL;
    TP_CALLBACK_ENVIRON CallBackEnviron;
    UINT i = 1000;
    UINT_PTR* args = (UINT_PTR*)HeapAlloc(GetProcessHeap(), 0, 2 * sizeof(UINT_PTR));
    args[0] = (UINT_PTR) lpLibFileName;
    args[1] = (UINT_PTR) AddrOf("LoadLibraryA", "kernel32.dll");
    InitializeThreadpoolEnvironment(&CallBackEnviron);
    work = CreateThreadpoolWork((PTP_WORK_CALLBACK) LoadLibraryThreadpoolCallback,
                                &args, 
                                &CallBackEnviron);
    SubmitThreadpoolWork(work);

    // Exponential backoff, though it should almost certainly be done before
    // the first iteration.
    while(i < 4000 && GetModuleHandleA(lpLibFileName) == NULL) {
        Sleep(i);
        i *= 2;
    }
}

void _InvokeLoadLibraryFls(LPCSTR lpLibFileName) {
    unsigned int flsIndex;
    UINT_PTR* args = (UINT_PTR*)HeapAlloc(GetProcessHeap(), 0, 2 * sizeof(UINT_PTR));
    args[0] = (UINT_PTR) lpLibFileName;
    args[1] = (UINT_PTR) AddrOf("LoadLibraryA", "kernel32.dll");
    flsIndex = FlsAlloc((PFLS_CALLBACK_FUNCTION) LoadLibraryFlsCallback);
    FlsSetValue(flsIndex, &args);
    FlsFree(flsIndex);
}

// This macro is what should actually be used to invoke LoadLibrary.
// It will choose between the two indirection methods, with selection based on
// the value of __COUNTER__, which is incremented by the compiler for each
// macro call.
#define InvokeLoadLibrary(lpLibFileName)                \
({                                                      \
    if (__COUNTER__ % 2 == 0)                             \
        _InvokeLoadLibraryTp(lpLibFileName);            \
    else                                                \
        _InvokeLoadLibraryFls(lpLibFileName);           \
})

#endif
