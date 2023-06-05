rule WinApiProxyXorOTP {
    meta:
        author = "micrictor"
        description = "Find the one-time-pad key and at least one of the encrypted strings"
    strings:
        $otp_key = {90 a7 31 f6 d4 42 23 e1 be e3 7c 0a 28 c8 f9 b0 cb e8 ad e3 9e da 3a 46}
        $encrypted_ntdll = {fe d3 55 9a b8 6c 47 8d d2}
        $encrypted_kernel32 = {fb c2 43 98 b1 2e 10 d3 90 87 10 66}
        $encrypted_ntallocvm = {de d3 70 9a b8 2d 40 80 ca 86 2a 63 5a bc 8c d1 a7 a5 c8 8e f1 a8 43}
        $encrypted_ntprotectvm = {de d3 61 84 bb 36 46 82 ca b5 15 78 5c bd 98 dc 86 8d c0 8c ec a3}
    condition:
        $otp_key and any of ($encrypted_ntdll, $encrypted_kernel32, $encrypted_ntallocvm, $encrypted_ntprotectvm)
}