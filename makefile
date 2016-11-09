ep: ep.o
	gcc -pthread -o ep ep.o

ep.o: ep.c
	gcc -c ep.c -Wall -pedantic -ansi -g

clean:
	rm -rf *.o
	rm -rf *~
	rm ep
