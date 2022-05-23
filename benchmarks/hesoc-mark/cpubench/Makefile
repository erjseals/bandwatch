CC=g++
CFLAGS=-O3 --std=gnu++11 -I.

all: cpubench 

cpubench: cpubench.cpp
	$(CC) cpubench.cpp $(CFLAGS) -o cpubench

clean:
	rm -f cpubench