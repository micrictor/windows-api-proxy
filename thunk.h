// header guards to prevent multiple inclusions of header
#ifndef THUNK_H
#define THUNK_H

#include <windows.h>

// JmpThunk takes an arbitrary number of 8-byte arguments, do the required
// register/stack shuffling, and then jump to the target address in the last
// argument.
// The first argument is the number of arguments for the target API.
__attribute__((always_inline)) inline void JmpThunk(
    unsigned int apiArgsCount, UINT_PTR args[]) {
    unsigned long long i; // Needs to be 64-bit value for the asm inline
    UINT_PTR value;
    UINT_PTR apiAddress = args[apiArgsCount];

    // Preserve RBX (used for iterator) and RSI on the stack
    asm inline("push %%rbx;"
               "push %%rsi;"
        : /* no output */
        : /* no input */
        : "rbx", "rsi");

    for (i = 0; i < apiArgsCount; i++) {
        // Store first argument in ECX, second in EDX, third in R8, fourth in R9,
        // and the rest on the stack.
        value = args[i];
        if (i == 0)
            asm inline("movq %0, %%rcx;"
                : /* no output */
                : "r"(value)
                : "rcx");
        else if (i == 1)
            asm inline("movq %0, %%rdx;"
                : /* no output */
                : "r"(value)
                : "rdx");
        else if (i == 2)
            asm inline("movq %0, %%r8;"
                : /* no output */
                : "r"(value)
                : "r8");
        else if (i == 3)
            asm inline("movq %0, %%r9;"
                : /* no output */
                : "r"(value)
                : "r9");
        // Push the rest of the arguments on the stack
        // The offset is 16 + i * 0x8, which accounts for the two preserved
        // 8-byte registers and the shadow store for the first 4 arguments.
        else {
            asm inline("movq %[value], 0x10(%%rsp, %[i], 0x8);"
                : /* no output */
                : [value] "g"(value), [i] "r"(i+1));
        }
    }
    // Restore RBX and RSI, and jump to the target address.
    asm inline(
        "pop %%rsi;"
        "pop %%rbx;"
        "jmp %0;"
        : /* no output */
        : "r"(apiAddress));
}


#endif