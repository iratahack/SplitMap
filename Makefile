#CFLAGS=-O3 -Wall -fsanitize=address -static-libasan -g
CC := clang
CFLAGS=-O3 -Wall

all: build/splitmap

clean:
	rm -f build/splitmap

build/splitmap: splitmap.c optimize.c compress.c memory.c
	mkdir -p build
	$(CC) $(CFLAGS) $+ -o build/splitmap
