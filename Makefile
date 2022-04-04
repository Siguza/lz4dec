.PHONY: all clean

all: lz4dec

lz4dec: src/lz4dec.S test/main.c
	$(CC) -o $@ $^ -Wall -O3 $(CFLAGS)

clean:
	rm -f lz4dec
