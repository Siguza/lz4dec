# lz4dec

LZ4 block decompressor written in arm64 assembly.

### Overview

lz4dec was written with a focus on small code size and is primarily intended for self-extracting firmware images. It has no dependencies, doesn't use the stack, and only does single-byte memory accesses, so it should work in any environment, with or without MMU, with any memory alignment. It also supports overlapping input and output buffers, so long as the output buffer doesn't "catch up" (i.e. output is never written into parts of the input buffer that haven't been parsed yet). Thus when in doubt, align the input to the end of available buffer.

It has two configuration:
- "Safe"  
  This is the default and should be used whenever feasible. In this configuration, the input is considered untrusted and proper bounds checks are employed.
- "Unsafe"  
  This configuration can be enabled by defining the `UNSAFE_LZ4` macro and will eliminate all bounds checks. This must only be used with trusted input, e.g. when combined with a "known-good" compressed firmware image.

At the time of writing, the code size of the safe configuration is 252 bytes while that of the unsafe configuration is 144 bytes.

### Usage

The [`lz4dec.S`](https://github.com/Siguza/lz4dec/blob/master/src/lz4dec.S) file exports a single function called `lz4dec`.

If `UNSAFE_LZ4` is not defined, then the prototype of that function is:

    extern uint64_t lz4dec(const void *src, void *dst, uint64_t srcsz, uint64_t dstsz);

If `UNSAFE_LZ4` is defined, then the prototype becomes:

    extern uint64_t lz4dec(const void *src, void *dst, uint64_t srcsz);

The return value is the number of bytes written to the output buffer, or `0` if an error occurred.

`srcsz` must only be the size of compressed data, trailing bytes that are not LZ4 block data are not allowed. This is required because the size must be known in order to determine where an LZ4 block ends.

### Testing

There is a test binary written in C that wraps around `lz4dec`.

On arm64 hosts, it can be built like so:

    make                        # for the safe configuration
    CFLAGS='-DUNSAFE_LZ4' make  # for the unsafe configuration

And used like so:

    ./lz4dec <file.lz4 >file

There are no command-line options.

I also wrote a [companion binary](https://github.com/Siguza/lz4hc) to generate single-block LZ4 data as used by this test binary.

### License

[MIT](https://github.com/Siguza/lz4dec/blob/master/LICENSE).
