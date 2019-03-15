CC = gcc
CFLAGS = -I.

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: multi_server.o
	$(CC) -o server multi_server.o

client: client.o
	$(CC) -o client client.o
