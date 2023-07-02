// Define import guards for the procaddr.h header.
#ifndef PROCADDR_H
#define PROCADDR_H
#include <windows.h>

// OTP-XOR the DLL names and function names to avoid static signatures.
unsigned char xor_key[] = 
"\x90\xa7\x31\xf6\xd4\x42\x23\xe1\xbe\xe3\x7c\x0a\x28\xc8\xf9\xb0\xcb\xe8\xad\xe3\x9e\xda\x3a\x46";

unsigned char NTDLL_DLL[] = "\xfe\xd3\x55\x9a\xb8\x6c\x47\x8d\xd2";
unsigned char KERNEL32_DLL[] = "\xfb\xc2\x43\x98\xb1\x2e\x10\xd3\x90\x87\x10\x66";
unsigned char NTALLOCATEVIRTUALMEMORY_NAME[] =
"\xde\xd3\x70\x9a\xb8\x2d\x40\x80\xca\x86\x2a\x63\x5a\xbc\x8c\xd1\xa7\xa5\xc8\x8e\xf1\xa8\x43";
unsigned char NTPROTECTVIRTUALMEMORY_NAME[] = 
"\xde\xd3\x61\x84\xbb\x36\x46\x82\xca\xb5\x15\x78\x5c\xbd\x98\xdc\x86\x8d\xc0\x8c\xec\xa3";
unsigned char HEAPFREE_NAME[] = "\xd8\xc2\x50\x86\x92\x30\x46\x84";
unsigned char HEAPALLOC_NAME[] = "\xd8\xc2\x50\x86\x95\x2e\x4f\x8e\xdd";
unsigned char INITONCEEXECUTEONCE_NAME[] = "\xd9\xc9\x58\x82\x9b\x2c\x40\x84\xfb\x9b\x19\x69\x5d\xbc\x9c\xff\xa5\x8b\xc8";
unsigned char TPALLOCWORK_NAME[] = "\xc4\xd7\x70\x9a\xb8\x2d\x40\xb6\xd1\x91\x17";
unsigned char TPPOSTWORK_NAME[] = "\xc4\xd7\x61\x99\xa7\x36\x74\x8e\xcc\x88";
unsigned char TPRELEASEWORK_NAME[] = "\xc4\xd7\x63\x93\xb8\x27\x42\x92\xdb\xb4\x13\x78\x43";

// XorDecrypt decrypts the xor_key using the xor_key.
#define XorDecrypt(inputBuf) ({                          \
    unsigned char *buf = inputBuf;                       \
    for (int i = 0; i < sizeof(inputBuf) - 1; i++) {     \
        buf[i] ^= xor_key[i % sizeof(xor_key)];          \
    }                                                    \
    buf;                                                 \
})

// AddrOf returns the address of a function in a module.
#define AddrOf(funcName, module) ((UINT_PTR)GetProcAddress(GetModuleHandleA(module), funcName))
#define NtAddrOf(funcName) ({            \
    AddrOf(funcName, "ntdll.dll");       \
})

#define EncAddrOf(encryptedFuncName, encryptedModule) ({                            \
    unsigned char *funcName = XorDecrypt(encryptedFuncName);                        \
    unsigned char *module = XorDecrypt(encryptedModule);                            \
    UINT_PTR result = (UINT_PTR)GetProcAddress(GetModuleHandleA(module), funcName); \
    XorDecrypt(encryptedFuncName);                        \
    XorDecrypt(encryptedModule);                            \
    result;                                                                         \
})
#define EncNtAddrOf(encryptedFuncName) ({          \
    EncAddrOf(encryptedFuncName, NTDLL_DLL);       \
})
#define EncKernelAddrOf(encryptedFuncName) ({   \
    EncAddrOf(encryptedFuncName, KERNEL32_DLL); \
})

#endif  // PROCADDR_H
