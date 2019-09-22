# Protocoale de comunicatii:
# Laborator 8: Multiplexare
# Makefile

CFLAGS = g++ -std=c++11 -g -Werror -Wall

# Portul pe care asculta serverul (de completat)
PORT = 

# Adresa IP a serverului (de completat)
IP_SERVER = 

all: server subscriber

# Compileaza server.c
server: server.cpp

# Compileaza subscriber.c
subscriber: subscriber.cpp

.PHONY: clean run_server run_subscriber

# Ruleaza serverul
run_server:
	./server ${PORT}

# Ruleaza clientul
run_subscriber:
	./subscriber ${IP_SERVER} ${PORT}

clean:
	rm -f server subscriber
