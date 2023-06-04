#ifndef ALLOCVM_H
#define ALLOCVM_H

#include <windows.h>
#include "procaddr.h"
#include "thunk.h"

UINT_PTR pNtAllocVM = (UINT_PTR)NULL;

__attribute__((naked)) PTP_WORK_CALLBACK AllocateVirtualMemoryThreadpoolCallback(
    PTP_CALLBACK_INSTANCE   Instance,
    UINT_PTR                *args[],
    PTP_WORK                 Work) {
    JmpThunk(6, *args);
}

__attribute__((naked)) PFLS_CALLBACK_FUNCTION AllocateVirtualMemoryFlsCallback(
    UINT_PTR *args[]) {
    JmpThunk(6, *args);
}

__attribute__((naked)) BOOL AllocateVirtualMemoryIOEOCallback(
    PINIT_ONCE  InitOnce,
    UINT_PTR    *args[],
    PVOID       *Context) {
    JmpThunk(6, *args);
}

void _InvokeAllocateVirtualMemoryTp(PVOID baseAddress, SIZE_T memoryLength) {
    PTP_WORK work = NULL;
    TP_CALLBACK_ENVIRON CallBackEnviron;
    UINT i = 1000;
    UINT_PTR* args = (UINT_PTR*)HeapAlloc(GetProcessHeap(), 0, 7 * sizeof(UINT_PTR));
    args[0] = (UINT_PTR) (HANDLE)-1;
    args[1] = (UINT_PTR) baseAddress;
    args[2] = (UINT_PTR) 0;
    args[3] = (UINT_PTR) &memoryLength;
    args[4] = (UINT_PTR) (MEM_COMMIT | MEM_RESERVE);
    args[5] = (UINT_PTR) PAGE_READWRITE;
    args[6] = (UINT_PTR) pNtAllocVM;
    InitializeThreadpoolEnvironment(&CallBackEnviron);
    work = CreateThreadpoolWork((PTP_WORK_CALLBACK) AllocateVirtualMemoryThreadpoolCallback,
                                &args, 
                                &CallBackEnviron);
    SubmitThreadpoolWork(work);

    // Exponential backoff, though it should almost certainly be done before
    // the first iteration.
    while(i < 4000 && baseAddress == NULL) {
        Sleep(i);
        i *= 2;
    }
    // Clean up, clean up, everybody clean up.
    // Free the args array, and close the threadpool.
    HeapFree(GetProcessHeap(), 0, args);
    CloseThreadpoolWork(work);
}

void _InvokeAllocateVirtualMemoryIOEO(PVOID baseAddress, SIZE_T memoryLength) {
    INIT_ONCE initOnce = INIT_ONCE_STATIC_INIT;
    UINT_PTR* args = (UINT_PTR*)HeapAlloc(GetProcessHeap(), 0, 7 * sizeof(UINT_PTR));
    args[0] = (UINT_PTR) (HANDLE)-1;
    args[1] = (UINT_PTR) baseAddress;
    args[2] = (UINT_PTR) 0;
    args[3] = (UINT_PTR) &memoryLength;
    args[4] = (UINT_PTR) (MEM_COMMIT | MEM_RESERVE);
    args[5] = (UINT_PTR) PAGE_READWRITE;
    args[6] = (UINT_PTR) pNtAllocVM;

    InitOnceExecuteOnce(&initOnce, (PINIT_ONCE_FN) AllocateVirtualMemoryIOEOCallback, &args, NULL);
    HeapFree(GetProcessHeap(), 0, args);
}


// This macro is what should actually be used to invoke AllocateVirtualMemory.
// It will choose between the two indirection methods, with selection based on
// the value of __COUNTER__, which is incremented by the compiler for each
// macro call.
#define InvokeAllocateVirtualMemory(baseAddress, memoryLength)                \
({                                                      \
    int value = __COUNTER__ % 2;                        \
    if (value == 0)                                     \
        _InvokeAllocateVirtualMemoryTp(baseAddress, memoryLength);            \
    else {                                              \
        _InvokeAllocateVirtualMemoryIOEO(baseAddress, memoryLength);      \
    }                                                   \
})

#endif
