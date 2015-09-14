
CC=gcc
CFLAGS=-O2 -Wall


BINS=libmyalloc.so

all: $(BINS)

libmyalloc.so: memory_shim.c
	$(CC) $(CFLAGS) -DNDEBUG -fPIC -shared -o libmyalloc.so memory_shim.c -lm
