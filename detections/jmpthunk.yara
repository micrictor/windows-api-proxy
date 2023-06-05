import "pe"
import "math"

rule WinApiProxyJmpThunk {
    meta:
        author = "micrictor"
        description = "Detect the JmpThunk in my Windows API Proxy repo"
    strings:
        // All the pushes at the start to preserve registers
        $thunk_prolog = {53 41 54 41 55 41 56 41 57 56 57}
        // All the pops to restore registers + jmp rax
        $thunk_epilog = {5F 5E 41 5F 41 5E 41 5D 41 5C 5B FF E0}
    condition:
        pe.is_64bit() // No point in doing heavier computations if it's not 64 bit
        and ($thunk_prolog and $thunk_epilog)
        and
        // Check that the prolog and epilog are within 200 bytes of each other
        for any i in (1..math.min(#thunk_prolog, 10)): (
            for any j in (1..math.min(#thunk_epilog, 10)): (
                @thunk_epilog[j] - @thunk_prolog[i] <= 200
                and @thunk_epilog[j] - @thunk_prolog[i] >= 0
            )
        )
        
}
