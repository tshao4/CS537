CC=gcc
CFLAGS=-g
OBJ=spin.o counter.o list.o hash.o
SO=libspin.so libcounter.so liblist.so libhash.so
DIR = $(shell pwd)

all: $(SO)

$(OBJ): %.o: %.c
	$(CC) -c $(CFLAGS) -fpic $<

libspin.so: spin.o
	$(CC) $(CFLAGS) -shared -o $@ $<
    
libcounter.so: counter.o
	$(CC) $(CFLAGS) -shared -o $@ -L$(DIR) -lspin $<
    
liblist.so: list.o
	$(CC) $(CFLAGS) -shared -o $@ -L$(DIR) -lspin $<
    
libhash.so: hash.o
	$(CC) $(CFLAGS) -shared -o $@ -L$(DIR) -lspin -llist $<

clean:
	rm -rf *.o *.so