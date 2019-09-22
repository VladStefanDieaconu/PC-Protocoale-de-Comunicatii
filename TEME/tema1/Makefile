all: send recv

link_emulator/lib.o:
	$(MAKE) -C link_emulator

send: send.o link_emulator/lib.o
	g++ -std=c++11 -g -Werror -Wall send.o link_emulator/lib.o -o send

recv: recv.o link_emulator/lib.o
	g++ -std=c++11 -g -Werror -Wall recv.o link_emulator/lib.o -o recv

.c.o:
	g++ -Wall -g -Werror -c $?

zip:
	zip dvs.zip *.cpp Makefile README

clean:
	$(MAKE) -C link_emulator clean
	-rm -f *.o send recv
