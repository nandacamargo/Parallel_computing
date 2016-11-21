CC = gcc
CFLAGS=-W -Wall -ansi -pedantic -lm -lpthread -fopenmp
DEPS = util.h
OBJ = util.o ep.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ep: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o *~ *~
