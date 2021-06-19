CC=g++
CFLAGS=-g -I.
DEPS = Socket.h Game.h
OBJ = Socket.o Game.o
LIBS=-lpthread

%.o: %.cc $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS)

all: Server Client

Server: $(OBJ) GameServer.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

Client: $(OBJ) GameClient.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)


.PHONY: clean

clean:
	rm -f *.o Server Client .Game.h.swp .Game.cc.swp

