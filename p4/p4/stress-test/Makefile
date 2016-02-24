CC=gcc
CFLAGS=-g #-DPTH

all : testCounter testHash testList

spin.o : spin.c
	$(CC) $(CFLAGS) -c -fpic spin.c

list.o : list.c
	$(CC) $(CFLAGS) -c -fpic list.c

hash.o : hash.c
	$(CC) $(CFLAGS) -c -fpic hash.c

counter.o : counter.c
	$(CC) $(CFLAGS) -c -fpic counter.c

libspin.so: spin.o
	$(CC) $(CFLAGS) -shared -o $@ $<

libhash.so : hash.o
	$(CC) $(CFLAGS) -shared -o $@ $<

libcounter.so : counter.o
	$(CC) $(CFLAGS) -shared -o $@ $<

liblist.so : list.o
	$(CC) $(CFLAGS) -shared -o $@ $<

testCounter : testcount.o libspin.so liblist.so libhash.so libcounter.so
	$(CC) $(CFLAGS) $< -o $@ -lhash -llist -lspin -L. -lpthread -lcounter

testList: testlist.o libspin.so liblist.so libhash.so 
	$(CC) $(CFLAGS) $< -o $@ -lhash -llist -lspin -L. -lpthread -lcounter

testHash: testhash.o libspin.so liblist.so libhash.so 
	$(CC) $(CFLAGS) $< -o $@ -lhash -llist -lspin -L. -lpthread -lcounter


clean :
	rm -rf *.o testCounter testList testHash *.so
