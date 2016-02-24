CC   = gcc

SERVER_SOURCES = server.c udp.c
CLIENT_SOURCES = client.c udp.c

all: client server

client: $(CLIENT_SOURCES)
	gcc -c -fpic client.c udp.c ;
	gcc -shared -o libmfs.so client.o udp.o

server: $(SERVER_SOURCES)
	$(CC) -o server $(SERVER_SOURCES)

clean:
	rm -f *.o *.so server

