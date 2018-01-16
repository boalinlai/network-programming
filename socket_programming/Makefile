CC=gcc 
CFLAGS=-Wall
all:client server
client: client.c
	$(CC) -o $@ $^

server: server.c
	$(CC) -o $@ $^
clean:
	rm server client	
