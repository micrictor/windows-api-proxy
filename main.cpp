#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <ostream>

#include "main.h"

int main() {
    PTP_WORK work = NULL;
    TP_CALLBACK_ENVIRON CallBackEnviron;
    LPVOID virtualMemoryAddress = NULL;
    SIZE_T memoryLength = 0xBEEF;

    // Create an instance of the IOEOExectutor class.
    IOEOExectutor ioExecutor("NtAllocateVirtualMemory", "ntdll.dll");
    std::cout << "IOEOExectutor created" << std::endl;
    ioExecutor.Call((HANDLE)-1,
                    (PVOID) &virtualMemoryAddress,
                    (ULONG_PTR) 0,
                    (PSIZE_T) &memoryLength,
                    (ULONG) MEM_COMMIT | MEM_RESERVE,
                    (ULONG) PAGE_READWRITE);

    while (virtualMemoryAddress == NULL) {
        putc('.', stdout);
        Sleep(100);
    }
    printf("\nAllocated memory location: %p\n", virtualMemoryAddress);

    return 0;
}
