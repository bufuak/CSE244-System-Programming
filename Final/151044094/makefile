all: server clients

server: server.o
	gcc server.o -o server -lpthread -lm

server.o: server.c
	gcc -c server.c

clients: clients.o
	gcc clients.o -o clients -lpthread -lm

clients.o: clients.c
	gcc -c clients.c

clean:
	rm *.o server clients
