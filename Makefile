CFLAGS=-g -Wall -Wextra -Wvla -fsanitize=address
CC=gcc

.PHONY: all clean

all: test.o mymalloc.o
	$(CC) -o out test.o mymalloc.o

test.o: test.c
	$(CC) -c $(CFLAGS) test.c -o test.o

mymalloc.o: mymalloc.c
	$(CC) -c $(CFLAGS) mymalloc.c -o mymalloc.o

clean:
	rm -f *.o
