#ifdef NO_GLOBAL
    .macro sym name
    L_\name:
    .endm
#else
#   define CONCAT(x,y) x ## y
    .macro sym name
    .globl CONCAT(__USER_LABEL_PREFIX__,\name)
    CONCAT(__USER_LABEL_PREFIX__,\name):
    .endm
#endif

// If UNSAFE_LZ4 is not defined:
// extern uint64_t lz4dec(const void *src, void *dst, uint64_t srcsz, uint64_t dstsz);

// If UNSAFE_LZ4 is defined:
// extern uint64_t lz4dec(const void *src, void *dst, uint64_t srcsz);

.p2align 2
sym lz4dec
    // Register allocation:
    // x0   next input byte ptr
    // x1   next output byte ptr
    // x2   end of input buffer
    // x3   end of output buffer
    // x4   literals length / matchlength
    // x5   scratch / offset
    // x6   scratch

    // x14  original start of output buffer
    // x15  return address
    mov x15, x30
    mov x14, x1
    // Calculate end of the buffer
    adds x2, x0, x2
#ifndef UNSAFE_LZ4
    // And make sure it's sane
    b.cs Lerr
    adds x3, x1, x3
    b.cs Lerr
#endif
Lsequence:
    // New sequence
#ifndef UNSAFE_LZ4
    cmp x0, x2
    b.hs Lerr
#endif
    ldrb w4, [x0], 1
    and w5, w4, 0xf
    // Extract literals length
    ubfx w4, w4, 4, 4
    cbz w4, Lmatchlength
    bl Llongsz
    // Copy literals to output buffer
#ifndef UNSAFE_LZ4
    // (x1 < x0 || x1 >= x2) && x0 < x2 && x1 < x3 && x4 <= (x2 - x0) && x4 <= (x3 - x1)
    cmp x1, x0
    ccmp x1, x2, 2, hs
    ccmp x0, x2, 2, hs
    ccmp x1, x3, 2, lo
    sub x6, x2, x0
    ccmp x4, x6, 2, lo
    sub x6, x3, x1
    ccmp x4, x6, 2, ls
    b.hi Lerr
#endif
Lliterals:
    ldrb w6, [x0], 1
    strb w6, [x1], 1
    sub x4, x4, 1
    cbnz x4, Lliterals
Lmatchlength:
    // End of the block only happens if matchlength is zero *and* we're at the
    // end of the input stream. If we're not at the end of the input stream,
    // then a matchlength of 0 means a copy of 4 bytes.
    cmp w5, 0
    ccmp x0, x2, 0, eq
    b.hs Lend
    mov w4, w5
    // Offset
#ifndef UNSAFE_LZ4
    sub x6, x2, x0
    cmp x6, 2
    b.lo Lerr
#endif
    ldrb w5, [x0], 1
    ldrb w6, [x0], 1
    bfi w5, w6, 8, 8
#ifndef UNSAFE_LZ4
    // Zero offset is invalid
    cbz w5, Lerr
#endif
    // Extract matchlength
    bl Llongsz
    adds x4, x4, 4
#ifndef UNSAFE_LZ4
    b.cs Lerr
#endif
    // Copy match
    subs x5, x1, x5
#ifndef UNSAFE_LZ4
    // Protect against underflow and OOB
    ccmp x5, x14, 0, hs
    sub x6, x3, x1
    ccmp x6, x4, 0, hs
    b.lo Lerr
#endif
Lmatch:
    ldrb w6, [x5], 1
    strb w6, [x1], 1
    sub x4, x4, 1
    cbnz x4, Lmatch
    b Lsequence

Llongsz:
    // Extract more size bytes
    cmp w4, 0xf
    b.ne Ldonesz
Lmoresz:
#ifndef UNSAFE_LZ4
    cmp x0, x2
    b.hs Lerr
#endif
    ldrb w6, [x0], 1
    adds x4, x4, x6
#ifndef UNSAFE_LZ4
    b.cs Lerr
#endif
    cmp w6, 0xff
    b.eq Lmoresz
Ldonesz:
    ret

#ifndef UNSAFE_LZ4
Lerr:
    mov x1, x14
#endif
Lend:
    sub x0, x1, x14
    ret x15
