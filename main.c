#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

#include "main.h"
#include "loadlibrary.h"
#include "thunk.h"

__attribute__((naked)) PTP_WORK_CALLBACK NtAllocateVirtualMemoryCallback(
    PTP_CALLBACK_INSTANCE       Instance,
    UINT_PTR                    *args[],
    PTP_WORK                    Work) {
    JmpThunk(6, *args);
}

int main() {
    PTP_WORK work = NULL;
    TP_CALLBACK_ENVIRON CallBackEnviron;
    LPVOID virtualMemoryAddress = NULL;
    SIZE_T memoryLength = 0xBEEF;

    // Allocate a 7-element array of 8-byte arguments on the heap
    UINT_PTR* args = (UINT_PTR*)HeapAlloc(GetProcessHeap(), 0, 7 * sizeof(UINT_PTR));
    args[0] = (UINT_PTR) (HANDLE)-1;
    args[1] = (UINT_PTR) &virtualMemoryAddress;
    args[2] = (UINT_PTR) 0;
    args[3] = (UINT_PTR) &memoryLength;
    args[4] = (UINT_PTR) (MEM_COMMIT | MEM_RESERVE);
    args[5] = (UINT_PTR) PAGE_EXECUTE_READ;
    args[6] = (UINT_PTR) NtAddrOf("NtAllocateVirtualMemory");

    // Create a threadpool to invoke our callback.
    // Because the callback is executed by a function inside of ntdll, the
    // callstack when we pass to NtAllocateVirtualMemory is "clean" (doesn't
    // include any function from user-defined code).
    InitializeThreadpoolEnvironment(&CallBackEnviron);
    work = CreateThreadpoolWork((PTP_WORK_CALLBACK) NtAllocateVirtualMemoryCallback,
                                &args, 
                                &CallBackEnviron);
    SubmitThreadpoolWork(work);

    while (virtualMemoryAddress == NULL) {
        putc('.', stdout);
        Sleep(100);
    }
    printf("\nAllocated memory location: %p\n", virtualMemoryAddress);

    // Just for funsies, load in a bunch of different images so our macro uses
    // all three methods.
    InvokeLoadLibrary("wininet.dll");
    printf("Loaded wininet.dll at %p\n", GetModuleHandleA("wininet.dll"));
    InvokeLoadLibrary("dbghelp.dll");
    printf("Loaded dbghelp.dll at %p\n", GetModuleHandleA("dbghelp.dll"));
    InvokeLoadLibrary("advapi32.dll");
    printf("Loaded advapi32.dll at %p\n", GetModuleHandleA("advapi32.dll"));
    InvokeLoadLibrary("crypt32.dll");
    printf("Loaded crypt32.dll at %p\n", GetModuleHandleA("crypt32.dll"));

    return 0;
}
