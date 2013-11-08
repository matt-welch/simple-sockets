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
	rm -f server client *.o core core.* *.gch

tidy: clean
	rm -f *.*~ *~

CONFIG		= -Wall -pedantic
