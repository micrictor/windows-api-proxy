## Windows API Proxy

A C library, using GCC features, for calling windows APIs with "clean" call stacks.

1. **[Why](#why)**
2. **[Proposed Detections](#proposed-detections)**
1. **[Techniques](#techniques)**
1. **[Thunk](#thunk)**
5. **[Macro Shenanigans](#macro-shenanigans)**


### Why

Endpoint Detection and Response (EDR) often inspects the call stacks for key
Windows API functions. They may do this by hooking the target procedures in
memory, either in userspace or kernelspace, or by leveraging the Event Tracing
for Windows (ETW) subsystem.

Developers of software used by threat actors know this. The authors of
[Cobalt Strike have written about it](https://www.cobaltstrike.com/blog/behind-the-mask-spoofing-call-stacks-dynamically-with-timers/), as has
[the author of Brute Ratel](https://0xdarkvortex.dev/proxying-dll-loads-for-hiding-etwti-stack-tracing/).

Detecting these techniques would protect EDR/AV users against these tools,
which have been used in attacks, as well as any more bespoke tools.

### Proposed Detections

Including the following features about a call stack should, in my opinion,
improve the accuracy of EDR/AV classification models for the detection of
potentially malicious software making API calls.
*   Whether or not the highest frame (most-recent call) originates from a 
    Windows signed DLL
    *   In "normal" callbacks that call Windows APIs, the callback function
        itself, most likely within user-defined code, would appear on the call
        stack.
    *   With a callback proxy, the highest frame is for a function inside
        ntdll/kernel32
*   The number of calls in the call stack
    *   A weak indicator, but a call stack when invoked via a callback is
        shorter than one directly called
*   Lifespan of the calling thread
    * Short-lived threads may be getting spun up and torn down to proxy APIs


Modifications to usermode hook(s) on functions of interest can also be tweaked
to detect `jmp`s into the function (rather than `call`) by checking register
values against the address of the function. If the matching register is `RAX`,
that isn't likely a strong signal, as `jmp rax` isn't an uncommon way to call
API functions in x64 Windows executables - and wouldn't aid with detecting my
implementation, because my thunk uses `jmp rax`. Other implementations
available freely online, however, use other registers, which may be a higher
fidelity indicator.

### Techniques

All of the techniques used in this repository rely on asychronous callbacks.
They all share the same limitation - while we can invoke methods with them,
we can't directly access the return values.

Each callback invocation method has a different amount of parameters it can
safely proxy to the target API. This is because we depend on the shadow space
and stack space for the internal method that invokes our callback for variable
storage.


| Technique         | Max # of Parameters |
|-------------------|---------------------|
| ThreadpoolWork    | 9 |
| FlsFree           | 4 |
| InvokeOnceRunOnce | 7 |

### Thunk

The magic of converting the supplied arguments into the proper registers/stack
offsets is all handled in `thunk.h`. Note that I make heavy use of inline
assembly to manipulate registers/memory. The comments in it might serve as a
pretty decent introduction to the Windows x64 calling convention.

There's two important things that make the callback methods and the thunk "play
nice," as far as avoiding adding our own functions onto the call stack.

First, the callback functions all have `__attribute__((naked))`, which tells
the compiler to not add a function prolog or epilog to the function. The prolog
is what would set up the stack, so dropping it from the function keeps our call
stack clean. It also means we can't have any allocated local variables, as those 
would be stored on the stack - and we won't have a stack frame to use for that.

Second, the thunk is forcibly inlined using `__attribute__((always_inline))`
and `inline`. This prevents our callback function from adding itself onto the
call stack when it calls our thunk.

### Macro Shenanigans

It's not really related to the primary purpose of this tool, but in
`loadlibrary.h` I built out a macro for calling LoadLibraryA via a proxy. It
uses the `__COUNTER__` macro, which increments by one each use, as a source of
pseudorandomness to select one of the three available proxying methods at
compile-time.


## Detections

I built `control.exe` that uses the exact same payload, exact same string
encryption method, and same dynamic import functionality.

The main difference between `control.exe` and `main.exe` is that `main.exe`
utilizes callbacks to indirectly do the VirtualAlloc and VirtualProtect calls.
`main.exe` also has some additional debug prints in the startup function.

control.exe, which just does direct execution, has 16/71 on VT.
https://www.virustotal.com/gui/file/99bcdbde638353fa59f2ce91c0ff7c27f7c2d5cbaf3f2cb720920f436316b8f4?nocache=1


![control virustotal detections](images/control.exe-2023-06-04.png?raw=true)

One day later, it has 28/71 on VT.


![control virustotal detections](images/control.exe-2023-06-05.png?raw=true)


main.exe, using the techniques described, has 5/71 on VT.
https://www.virustotal.com/gui/file/f2f44f72fd1f12bf184327e1a9a79e65eb8b100146ccbe73749f41a41084fbd2?nocache=1


![main virustotal detections](images/main.exe-2023-06-04.png?raw=true)

Note that while Elastic still detects it, confidence goes from "high" to "moderate".

One day later, it has 6/71.


![main virustotal detections](images/main.exe-2023-06-05.png?raw=true)



With encrypted function names, 3/71 - https://www.virustotal.com/gui/file/e9b6b897f0a64db8506954d73ed4b4528601afcee7cc549ae7397c32591e2f4c/behavior

