CC=g++
CFLAGS=-g -std=c++1y -I.
DEPS = Socket.h Game.h
OBJ = Socket.o Game.o
LIBS=-lpthread

%.o: %.cc $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS)

all: cs cc

cs: $(OBJ) GameServer.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

cc: $(OBJ) GameClient.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)


.PHONY: clean

clean:
	rm -f *.o cs cc

