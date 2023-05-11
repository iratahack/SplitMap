#CFLAGS=-O3 -Wall -fsanitize=address -static-libasan -g
CC := clang
CFLAGS=-O3 -Wall

all: splitmap

clean:
	rm -f *.o splitmap

splitmap: splitmap.c optimize.c compress.c memory.c
	$(CC) $(CFLAGS) $+ -o splitmap
