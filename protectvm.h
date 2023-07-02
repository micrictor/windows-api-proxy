#ifndef PROTECTVM_H
#define PROTECTVM_H

#include <windows.h>
#include "procaddr.h"
#include "thunk.h"

UINT_PTR pNtProtectVM = (UINT_PTR)NULL;
typedef LPVOID(NTAPI *HeapAlloc_t)(HANDLE, DWORD, SIZE_T);
typedef LPVOID(NTAPI *HeapFree_t)(HANDLE, DWORD, LPVOID);
typedef VOID(NTAPI *TpAllocWork_t)(PTP_WORK*, PTP_WORK_CALLBACK, PVOID, PTP_CALLBACK_ENVIRON);
typedef VOID(NTAPI *TpPostWork_t)(PTP_WORK);
typedef VOID(NTAPI *TpReleaseWork_t)(PTP_WORK);

__attribute__((naked)) PTP_WORK_CALLBACK ProtectVirtualMemoryThreadpoolCallback(
    PTP_CALLBACK_INSTANCE   Instance,
    UINT_PTR                *args[],
    PTP_WORK                 Work) {
    JmpThunk(5, *args);
}

void _InvokeProtectVirtualMemoryTp(PVOID *baseAddress, PSIZE_T memoryLength, ULONG newProtect, ULONG *oldProtect) {
    HeapAlloc_t lHeapAlloc = (HeapAlloc_t)EncKernelAddrOf(HEAPALLOC_NAME);
    HeapFree_t lHeapFree = (HeapFree_t)EncKernelAddrOf(HEAPFREE_NAME);
    TpAllocWork_t lTpAllocWork = (TpAllocWork_t)EncNtAddrOf(TPALLOCWORK_NAME);
    TpPostWork_t lTpPostWork = (TpPostWork_t)EncNtAddrOf(TPPOSTWORK_NAME);
    TpReleaseWork_t lTpReleaseWork = (TpReleaseWork_t)EncNtAddrOf(TPRELEASEWORK_NAME);

    PTP_WORK work = NULL;
    TP_CALLBACK_ENVIRON CallBackEnviron;
    UINT i = 1000;
    UINT_PTR* args = (UINT_PTR*)lHeapAlloc(GetProcessHeap(), 0, 6 * sizeof(UINT_PTR));
    args[0] = (UINT_PTR) (HANDLE)-1;
    args[1] = (UINT_PTR) baseAddress;
    args[2] = (UINT_PTR) memoryLength;
    args[3] = (UINT_PTR) newProtect;
    args[4] = (UINT_PTR) oldProtect;
    args[5] = (UINT_PTR) pNtProtectVM;
    InitializeThreadpoolEnvironment(&CallBackEnviron);
    lTpAllocWork(&work, (PTP_WORK_CALLBACK) ProtectVirtualMemoryThreadpoolCallback,
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

#define InvokeProtectVirtualMemory(baseAddress, memoryLength, newProtect, oldProtect)                \
({                                                                                                    \
    _InvokeProtectVirtualMemoryTp(baseAddress, memoryLength, newProtect, oldProtect);                \
})

#endif
