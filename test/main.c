#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef UNSAFE_LZ4
extern uint64_t lz4dec(const void *src, void *dst, uint64_t srcsz, uint64_t dstsz);
#else
extern uint64_t lz4dec(const void *src, void *dst, uint64_t srcsz);
#endif

int main(void)
{
    void *inbuf = NULL;
    size_t sz = 0x8000;
    size_t len = 0;
    while(1)
    {
        sz *= 2;
        inbuf = realloc(inbuf, sz);
        if(!inbuf)
        {
            fprintf(stderr, "realloc: %s\n", strerror(errno));
            return -1;
        }
        size_t want = sz - len;
        size_t have = fread((char*)inbuf + len, 1, sz - len, stdin);
        len += have;
        if(have < want)
        {
            if(feof(stdin))
            {
                break;
            }
            fprintf(stderr, "fread: %s\n", strerror(errno));
            return -1;
        }
    }

    // Docs say max compression ratio is around 250, so with 256
    // we should be good even if we don't know the actual output size?
    size_t outsz = len * 0x100;
    void *outbuf = malloc(outsz);
    if(!outbuf)
    {
        fprintf(stderr, "malloc: %s\n", strerror(errno));
        return -1;
    }

#ifndef UNSAFE_LZ4
    uint64_t outlen = lz4dec(inbuf, outbuf, len, outsz);
#else
    uint64_t outlen = lz4dec(inbuf, outbuf, len);
#endif

    if(!outlen)
    {
        fprintf(stderr, "lz4 error\n");
        return -1;
    }

    size_t written = fwrite(outbuf, 1, outlen, stdout);
    if(written != outlen)
    {
        fprintf(stderr, "fwrite: %s\n", strerror(errno));
        return -1;
    }

    fprintf(stderr, "Decompressed 0x%zx bytes to 0x%llx bytes\n", len, (unsigned long long)outlen);
    return 0;
}
