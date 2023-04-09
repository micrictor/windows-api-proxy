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
    // Preserve RBX, R12, R13, R14, R15, RSI, and RDI.
    // These are the callee-saved registers, and we don't want to clobber them.
    asm inline(
        "push %%rbx;"
        "push %%r12;"
        "push %%r13;"
        "push %%r14;"
        "push %%r15;"
        "push %%rsi;"
        "push %%rdi;"
        : /* no output */
        : /* no input */
        : "rbx", "r12", "r13", "r14", "r15", "rsi", "rdi");
    unsigned long long i;
    unsigned long long value;
    // jmp rax is the standard way of jumping to a register address.
    // RAX is also guaranteed to be caller-saved, so no risk of breakage.
    register unsigned long long apiAddress asm("rax") = args[apiArgsCount];

    // Store each parameter as specified by the Windows x64 Calling Convention.
    // https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention?view=vs-2019
    // RCX, RDX, R8, R9, and the rest on the stack.
    for (i = 0; i < apiArgsCount; i++) {
        value = args[i];
        // Store the register values on the stack, we'll go get them later.
        if (i <= 3)
            asm inline(
                "movq %%rsp, %%rdi;"
                "subq %[i], %%rdi;"
                "movq %[value],  (%%rdi);"
                : /* no output */
                : [value] "g"(value), [i] "g"((i+1) * 8)
                : "rdi");
        // Push the rest of the arguments on the stack
        // The offset is (7*8) + i * 0x8, which accounts for the 7 preserved
        // 8-byte registers and the shadow store for the first 4 arguments.
        else {
            asm inline("movq %[value], 0x38(%%rsp, %[i], 0x8);"
                : /* no output */
                : [value] "g"(value), [i] "r"(i+1));
        }
    }
    // Grab our register values from the stack.
    // Note that this may end up filling the argument registers with garbage,
    // but that's OK because they'll just get ignored/cleaned up.
    asm inline(
        "movq -0x08(%%rsp), %%rcx;"
        "movq -0x10(%%rsp), %%rdx;"
        "movq -0x18(%%rsp), %%r8;"
        "movq -0x20(%%rsp), %%r9;"
        : /* no output */
        : /* no input */
        : "rcx", "rdx", "r8", "r9");
    // Restore callee-cleanup registers, and jump to the target address.
    asm inline(
        "pop %%rdi;"
        "pop %%rsi;"
        "pop %%r15;"
        "pop %%r14;"
        "pop %%r13;"
        "pop %%r12;"
        "pop %%rbx;"
        "jmp %0;"
        : /* no output */
        : "r"(apiAddress)
        : "rbx", "r12", "r13", "r14", "r15", "rsi", "rdi");
}


#endif