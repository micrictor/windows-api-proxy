#ifndef MAIN_H
#define MAIN_H
#include <windows.h>
#include "thunk.h"

#define AddrOf(funcName, module) ((UINT_PTR)GetProcAddress(GetModuleHandleA(module), funcName))
#define NtAddrOf(funcName) (AddrOf(funcName, "ntdll.dll"))

typedef SC_HANDLE (*FP_OPENSCMANAGER)(LPCSTR, LPCSTR, DWORD);
typedef SC_HANDLE (*FP_OPENSERVICE)(SC_HANDLE, LPCSTR, DWORD);

#endif  // MAIN_H