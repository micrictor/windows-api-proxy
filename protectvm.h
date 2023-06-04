#ifndef PROTECTVM_H
#define PROTECTVM_H

#include <windows.h>
#include "procaddr.h"
#include "thunk.h"

UINT_PTR pNtProtectVM = (UINT_PTR)NULL;

__attribute__((naked)) PTP_WORK_CALLBACK ProtectVirtualMemoryThreadpoolCallback(
    PTP_CALLBACK_INSTANCE   Instance,
    UINT_PTR                *args[],
    PTP_WORK                 Work) {
    JmpThunk(5, *args);
}

__attribute__((naked)) BOOL ProtectVirtualMemoryIOEOCallback(
    PINIT_ONCE  InitOnce,
    UINT_PTR    *args[],
    PVOID       *Context) {
    JmpThunk(5, *args);
}

void _InvokeProtectVirtualMemoryTp(PVOID *baseAddress, PSIZE_T memoryLength, ULONG newProtect, ULONG *oldProtect) {
    PTP_WORK work = NULL;
    TP_CALLBACK_ENVIRON CallBackEnviron;
    UINT i = 1000;
    UINT_PTR* args = (UINT_PTR*)HeapAlloc(GetProcessHeap(), 0, 6 * sizeof(UINT_PTR));
    args[0] = (UINT_PTR) (HANDLE)-1;
    args[1] = (UINT_PTR) baseAddress;
    args[2] = (UINT_PTR) memoryLength;
    args[3] = (UINT_PTR) newProtect;
    args[4] = (UINT_PTR) oldProtect;
    args[5] = (UINT_PTR) pNtProtectVM;
    InitializeThreadpoolEnvironment(&CallBackEnviron);
    work = CreateThreadpoolWork((PTP_WORK_CALLBACK) ProtectVirtualMemoryThreadpoolCallback,
                                &args, 
                                &CallBackEnviron);
    SubmitThreadpoolWork(work);

    // Exponential backoff, though it should almost certainly be done before
    // the first iteration.
    while(i < 4000 && oldProtect == NULL) {
        Sleep(i);
        i *= 2;
    }
    // Clean up, clean up, everybody clean up.
    // Free the args array, and close the threadpool.
    HeapFree(GetProcessHeap(), 0, args);
    CloseThreadpoolWork(work);
}

// Idk why but this one only works with threadpools and not IOEO
#define InvokeProtectVirtualMemory(baseAddress, memoryLength, newProtect, oldProtect)                \
({                                                                                                    \
    _InvokeProtectVirtualMemoryTp(baseAddress, memoryLength, newProtect, oldProtect);                \
})

#endif
