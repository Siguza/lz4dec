#include <stdint.h>

// C version just to see if clang can produce better asm.
// As of now, it hasn't.

uint64_t lz4dec(const void *src, void *dst, uint64_t srcsz, uint64_t dstsz)
{
    const uint8_t *x0 = src;
    uint8_t *x1 = dst;
    uintptr_t x2p;
    if(__builtin_add_overflow((uintptr_t)x0, srcsz, &x2p)) return 0;
    const uint8_t *x2 = (const uint8_t*)x2p;
    uintptr_t x3p;
    if(__builtin_add_overflow((uintptr_t)x1, dstsz, &x3p)) return 0;
    uint8_t *x3 = (uint8_t*)x3p;
    while(1)
    {
        uint8_t w6;
        if(x0 >= x2) return 0;
        uint8_t token = *x0++;
        uint64_t litlen = token >> 4;
        uint64_t matchlen = token & 0xf;
        if(litlen == 0xf)
        {
            do
            {
                if(x0 >= x2) return 0;
                w6 = *x0++;
                if(__builtin_add_overflow(litlen, w6, &litlen)) return 0;
            } while(w6 == 0xff);
        }
        if(!((x1 < x0 || x1 >= x2) && x0 < x2 && x1 < x3 && litlen <= (x2 - x0) && litlen <= (x3 - x1))) return 0;
        for(; litlen > 0; --litlen)
        {
            *x1++ = *x0++;
        }
        if(matchlen == 0 && x0 >= x2) return x1 - (uint8_t*)dst;
        if(x2 - x0 < 2) return 0;
        uint16_t offset = (uint16_t)x0[0] | ((uint16_t)x0[1] << 8);
        x0 += 2;
        if(!offset) return 0;
        if(matchlen == 0xf)
        {
            do
            {
                if(x0 >= x2) return 0;
                w6 = *x0++;
                if(__builtin_add_overflow(matchlen, w6, &matchlen)) return 0;
            } while(w6 == 0xff);
        }
        if(__builtin_add_overflow(matchlen, 4, &matchlen)) return 0;
        uintptr_t x5p;
        if(__builtin_sub_overflow((uintptr_t)x1, offset, &x5p)) return 0;
        uint8_t *x5 = (uint8_t*)x5p;
        if(x5 < (uint8_t*)dst || matchlen > (x3 - x1)) return 0;
        for(; matchlen > 0; --matchlen)
        {
            *x1++ = *x5++;
        }
    }
}
