all: exe

exe: main.o listdir.o list.o
	gcc main.o list.o listdir.o -o exe

main.o: main.c
	gcc -c main.c

listdir.o: list.c listdir.c
	gcc -c list.c listdir.c

clean:
	rm *.o exe
