CC=gcc
CFLAGS=-g

all : libmem.so

mem.o : mem.c
	$(CC) $(CFLAGS) -c -fpic mem.c -O3

libmem.so: mem.o
	$(CC) $(CFLAGS) -shared -o $@ $<

clean :
	rm -rf *.o *.so