CC=g++
CFLAGS=-O3 --std=gnu++11 -I.

all: membench meminterf

membench: membench.cpp
	$(CC) membench.cpp $(CFLAGS) -o membench

meminterf: meminterf.cpp
	$(CC) meminterf.cpp $(CFLAGS) -o meminterf	

clean:
	rm -f membench
	rm -f meminterf

