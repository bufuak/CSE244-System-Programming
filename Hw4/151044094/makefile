all: grepTh

grepTh: main.o listdir.o list.o
	gcc main.o list.o listdir.o -o grepTh -lpthread

main.o: main.c
	gcc -c main.c

listdir.o: list.c listdir.c
	gcc -c list.c listdir.c

clean:
	rm *.o grepTh
