#ifndef ALLOCVM_H
#define ALLOCVM_H

#include <windows.h>
#include "procaddr.h"
#include "thunk.h"

UINT_PTR pNtAllocVM = (UINT_PTR)NULL;

typedef LPVOID(NTAPI *HeapAlloc_t)(HANDLE, DWORD, SIZE_T);
typedef LPVOID(NTAPI *HeapFree_t)(HANDLE, DWORD, LPVOID);
typedef VOID(NTAPI *TpAllocWork_t)(PTP_WORK*, PTP_WORK_CALLBACK, PVOID, PTP_CALLBACK_ENVIRON);
typedef VOID(NTAPI *TpPostWork_t)(PTP_WORK);
typedef VOID(NTAPI *TpReleaseWork_t)(PTP_WORK);
typedef BOOL(NTAPI *IOEO_t)(PINIT_ONCE, PINIT_ONCE_FN, PVOID, LPVOID);

__attribute__((naked)) PTP_WORK_CALLBACK AllocateVirtualMemoryThreadpoolCallback(
    PTP_CALLBACK_INSTANCE   Instance,
    UINT_PTR                *args[],
    PTP_WORK                 Work) {
    JmpThunk(6, *args);
}

__attribute__((naked)) BOOL AllocateVirtualMemoryIOEOCallback(
    PINIT_ONCE  InitOnce,
    UINT_PTR    *args[],
    PVOID       *Context) {
    JmpThunk(6, *args);
}

void _InvokeAllocateVirtualMemoryTp(PVOID baseAddress, SIZE_T memoryLength) {
    HeapAlloc_t lHeapAlloc = (HeapAlloc_t)EncKernelAddrOf(HEAPALLOC_NAME);
    HeapFree_t lHeapFree = (HeapFree_t)EncKernelAddrOf(HEAPFREE_NAME);
    TpAllocWork_t lTpAllocWork = (TpAllocWork_t)EncNtAddrOf(TPALLOCWORK_NAME);
    TpPostWork_t lTpPostWork = (TpPostWork_t)EncNtAddrOf(TPPOSTWORK_NAME);
    TpReleaseWork_t lTpReleaseWork = (TpReleaseWork_t)EncNtAddrOf(TPRELEASEWORK_NAME);

    printf("%p\n", lHeapAlloc);
    printf("%p\n", lHeapFree);
    printf("%p\n", lTpAllocWork);
    printf("%p\n", lTpPostWork);
    printf("%p\n", lTpReleaseWork);

    PTP_WORK work = NULL;
    TP_CALLBACK_ENVIRON CallBackEnviron;
    UINT i = 1000;
    UINT_PTR* args = (UINT_PTR*)lHeapAlloc(GetProcessHeap(), 0, 7 * sizeof(UINT_PTR));
    args[0] = (UINT_PTR) (HANDLE)-1;
    args[1] = (UINT_PTR) baseAddress;
    args[2] = (UINT_PTR) 0;
    args[3] = (UINT_PTR) &memoryLength;
    args[4] = (UINT_PTR) (MEM_COMMIT | MEM_RESERVE);
    args[5] = (UINT_PTR) PAGE_READWRITE;
    args[6] = (UINT_PTR) pNtAllocVM;
    InitializeThreadpoolEnvironment(&CallBackEnviron);
    lTpAllocWork(&work, (PTP_WORK_CALLBACK) AllocateVirtualMemoryThreadpoolCallback,
                                &args, 
                                &CallBackEnviron);
    lTpPostWork(work);

    // Exponential backoff, though it should almost certainly be done before
    // the first iteration.
    while(i < 4000) {
        Sleep(i);
        i *= 2;
    }
    // Clean up, clean up, everybody clean up.
    // Free the args array, and close the threadpool.
    lHeapFree(GetProcessHeap(), 0, args);
    lTpReleaseWork(work);
}

void _InvokeAllocateVirtualMemoryIOEO(PVOID baseAddress, SIZE_T memoryLength) {
    HeapAlloc_t lHeapAlloc = (HeapAlloc_t)EncKernelAddrOf(HEAPALLOC_NAME);
    HeapFree_t lHeapFree = (HeapFree_t)EncKernelAddrOf(HEAPFREE_NAME);
    IOEO_t lIOEO = (IOEO_t)EncKernelAddrOf(INITONCEEXECUTEONCE_NAME);

    INIT_ONCE initOnce = INIT_ONCE_STATIC_INIT;
    UINT_PTR* args = (UINT_PTR*)lHeapAlloc(GetProcessHeap(), 0, 7 * sizeof(UINT_PTR));
    args[0] = (UINT_PTR) (HANDLE)-1;
    args[1] = (UINT_PTR) baseAddress;
    args[2] = (UINT_PTR) 0;
    args[3] = (UINT_PTR) &memoryLength;
    args[4] = (UINT_PTR) (MEM_COMMIT | MEM_RESERVE);
    args[5] = (UINT_PTR) PAGE_READWRITE;
    args[6] = (UINT_PTR) pNtAllocVM;

    InitOnceExecuteOnce(&initOnce, (PINIT_ONCE_FN) AllocateVirtualMemoryIOEOCallback, &args, NULL);
    lHeapFree(GetProcessHeap(), 0, args);
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
