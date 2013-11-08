simple-sockets
==============

Simple sockets client/server for CSE434: Networks

Algorithm for client: 
1) establish a connection to the server
	socket(), sendto(), recvfrom()
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
2) connect to a client
3) receive structure from the client 
	3a) take the character received from the client and prepend it to the
		server's string, discarding the last character in the string (rotate
		characters)
	3b) 



