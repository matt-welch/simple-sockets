# CSE434: simple sockets server and client
# Jesse Wuale and Matt Welch 
#
#

all: server client

server: UDPServer.o request.hpp
	g++ -o server UDPServer.o

client: UDPClient.o request.hpp
	g++ -o client UDPClient.o

UDPServer.o: UDPServer.cpp request.hpp UDPServer.hpp
	g++ -c $(CONFIG) UDPServer.cpp  request.hpp UDPServer.hpp

UDPClient.o: UDPClient.cpp request.hpp
	g++ -c $(CONFIG) UDPClient.cpp  request.hpp

clean:
	rm -f server client inc.txt *.o core core.* *.gch
	ctags --c-kinds=+defglmpstux -R

tidy: clean
	rm -f *.*~ *~


DEBUG_FLAGS = -g3 -ggdb -O0 -Wall -pedantic -DDEBUG
CONFIG		= -Wall -pedantic

debug: CONFIG=$(DEBUG_FLAGS)
debug: client server

run: server client
	./server 65432
