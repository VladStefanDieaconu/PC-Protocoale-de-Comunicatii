all: link lib.o

link: link.o queue.o
	gcc -g link.o queue.o -o link -O3 -pthread

.c.o: 
	gcc -Wall -g -c $? -O3 -pthread

clean:
	rm -f *.o link
