all: timerServer seeWhat showResults

timerServer: timerServer.o
	gcc timerServer.o -o timerServer -lm

seeWhat: seeWhat.o
	gcc seeWhat.o -o seeWhat -lm	

showResults: showResults.o
	gcc showResults.o -o showResults -lm

timerServer.o: timerServer.c
	gcc -c timerServer.c

seeWhat.o: seeWhat.c
	gcc -c seeWhat.c

showResults.o: showResults.c
	gcc -c showResults.c
clean:
	rm *.o timerServer seeWhat showResults
