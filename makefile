ep: ep.o
	gcc -pthread -lm -o ep ep.o

ep.o: ep.c
	gcc -c ep.c -Wall -pedantic -ansi -g

clean:
	rm -rf *.o
	rm -rf *~
	rm ep
