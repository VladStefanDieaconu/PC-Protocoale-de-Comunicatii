all: send recv 

send: send.o link_emulator/lib.o
	gcc -g send.o link_emulator/lib.o -o send

recv: recv.o link_emulator/lib.o
	gcc -g recv.o link_emulator/lib.o -o recv

.c.o: 
	gcc -Wall -g -c $? 

clean:
	rm -f *.o send recv 
