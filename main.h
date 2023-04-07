#ifndef MAIN_H
#define MAIN_H
#include <windows.h>
#include "thunk.h"

#define AddrOf(funcName, module) ((UINT_PTR)GetProcAddress(GetModuleHandleA(module), funcName))
#define NtAddrOf(funcName) (AddrOf(funcName, "ntdll.dll"))

// NtAllocateVirtualMemoryCallback is the callback function for a thread pool worker.
// It calls NtAllocateVirtualMemory with the arguments passed to the thread pool worker.
__attribute__((naked)) PTP_WORK_CALLBACK NtAllocateVirtualMemoryCallback(
    PTP_CALLBACK_INSTANCE       Instance,
    UINT_PTR                    *args[],
    PTP_WORK                    Work);

#endif  // MAIN_H