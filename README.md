simple-sockets
==============

Simple sockets client/server for CSE434: Networks

Algorithm for client: 
1) establish a connection to the server
	socket(), sendto(), recvfrom()
1a) determine the request number when failures will begin (rand(1-20))
2) issue 20 requests to the server
	2a) randomly select a character in the alphabet
	2b) keep sending the request after a timeout until get a response
	2c) begin failing after a random number of requests between 1 and 20 with
		a failure probability of (p=0.5) 
		2c1) failure is represented by incrementing the incarnation number
		2c2) incarnation number is stored in a file which should be locked and
			unlocked on accesses.

Algorithm for Server: 
Server maintains a 5-character log of the last 5 requests 

1) Establish a listening socket
	maintain a table of the requests from each client 
		map to vector of resuests?
2) receive a connection from a client
3) receive structure from the client 
	3a) take the character received from the client and prepend it to the
		server's string, discarding the last character in the string (rotate
		characters)
	3b) fail in two ways
		3b1) drop the request without performing the request (10%)
		3b2) perform the request and then drop the request (10%)
		3b3) if no fail, print the client and the string to stdout



