# #########################################################
# FILENAME:    Makefile
# NAME:        Makefile for UDP Simple Sockets client/server program
# AUTHORS:     Jesse Quale, Matt Welch
# SCHOOL:      Arizona State University
# CLASS:       CSE434: Introduction to Networks
# INSTRUCTOR:  Dr. Violet Syrotiuk
# SECTION:     71101
# TERM:        Fall 2013
# DESCRIPTION: Makefile with config and debug options for simple-sockets
# 	client/server program. run and runclient targets assume running on the ASU
# 	general1 server	 
# #########################################################

all: server client

server: UDPServer.o request.hpp
	g++ -o UDPServer UDPServer.o

client: UDPClient.o request.hpp
	g++ -o UDPClient UDPClient.o

UDPServer.o: UDPServer.cpp request.hpp UDPServer.hpp
	g++ -c $(CONFIG) UDPServer.cpp  request.hpp UDPServer.hpp

UDPClient.o: UDPClient.cpp request.hpp
	g++ -c $(CONFIG) UDPClient.cpp  request.hpp

clean:
	rm -f UDPServer UDPClient inc.txt *.o core core.* *.gch
	ctags --c-kinds=+defglmpstux -R

tidy: clean
	rm -f *.*~ *~ *.tmp


DEBUG_FLAGS = -g3 -ggdb -O0 -Wall -pedantic -DDEBUG
CONFIG		= -Wall -pedantic

debug: CONFIG=$(DEBUG_FLAGS)
debug: client server

verbose: CONFIG=$(DEBUG_FLAGS) -DVERBOSE
verbose: client server

run: all
	clear
	./UDPServer 65432 | tee output_server.tmp

runclient: all 
	./UDPClient 129.219.10.241 65432 0 | tee output_client.tmp
