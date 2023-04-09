#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

#include "main.h"
#include "thunk.h"

__attribute__((naked)) PFLS_CALLBACK_FUNCTION LoadLibraryCallback(
    UINT_PTR *args[]) {
    JmpThunk(1, *args);
}

__attribute__((naked)) PFLS_CALLBACK_FUNCTION OpenServiceCallback(
    UINT_PTR *args[]) {
    JmpThunk(4, *args);
}

int main() {
    DWORD flsIndex;
    SC_HANDLE hSCManager;
    SC_HANDLE hService;
    LPDWORD  lpcchBuffer;

    // The FLS callback function is doesn't have any stack-space, so we can only
    // pass up to 4 arguments to our API call with it.
    UINT_PTR* args = (UINT_PTR*)HeapAlloc(GetProcessHeap(), 0, 2 * sizeof(UINT_PTR));
    args[0] = (UINT_PTR) "advapi32.dll";
    args[1] = (UINT_PTR) AddrOf("LoadLibraryA", "kernel32.dll");
    flsIndex = FlsAlloc(LoadLibraryCallback);
    FlsSetValue(flsIndex, &args);
    FlsFree(flsIndex);
    printf("Loaded advapi32.dll at %p\n", GetModuleHandleA("advapi32.dll"));

    // Now that our module is loaded, we can start to take useful action.
    // We can't use callbacks to get a handle to a service, so we call those by address.
    hSCManager = ((FP_OPENSCMANAGER)(AddrOf("OpenSCManagerA", "advapi32.dll")))(NULL, NULL, SC_MANAGER_CONNECT);
    printf("Opened service manager at %p\n", hSCManager);
    hService = ((FP_OPENSERVICE)(AddrOf("OpenServiceA", "advapi32.dll")))(hSCManager, "WinDefend", SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS);
    printf("Opened service at %p\n", hService);
    // We can, though, indirectly query the status of a single service (like windows defender)
    LPQUERY_SERVICE_CONFIGA lpServiceConfig = (LPQUERY_SERVICE_CONFIGA)HeapAlloc(GetProcessHeap(), 0, 0x1000);
    UINT_PTR* args2 = (UINT_PTR*)HeapAlloc(GetProcessHeap(), 0, 5 * sizeof(UINT_PTR));
    args[0] = (UINT_PTR) hService;
    args[1] = (UINT_PTR) lpServiceConfig;
    args[2] = (UINT_PTR) 0x1000;
    args[3] = (UINT_PTR) &lpcchBuffer;
    args[4] = (UINT_PTR) AddrOf("QueryServiceConfigA", "advapi32.dll");

    flsIndex = FlsAlloc(OpenServiceCallback);
    FlsSetValue(flsIndex, &args);
    FlsFree(flsIndex);
    // Print the service path and start type as a human-readable string.
    printf("Service path: %s\n", lpServiceConfig->lpBinaryPathName);
    switch (lpServiceConfig->dwStartType) {
        case SERVICE_AUTO_START:
            printf("Service start type: Auto\n");
            break;
        case SERVICE_BOOT_START:
            printf("Service start type: Boot\n");
            break;
        case SERVICE_DEMAND_START:
            printf("Service start type: Demand\n");
            break;
        case SERVICE_DISABLED:
            printf("Service start type: Disabled\n");
            break;
        case SERVICE_SYSTEM_START:
            printf("Service start type: System\n");
            break;
        default:
            printf("Service start type: Unknown\n");
            break;
    }


    return 0;
}
