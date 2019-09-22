all: link lib.o

link: link.o queue.o
	gcc -g link.o queue.o -o link -lpthread

.c.o: 
	gcc -Wall -g -c $? -lpthread

clean:
	-rm *.o link
