# FILENAME:    README.md
# NAME:        readme for UDP Simple Sockets client and server programs
# AUTHORS:     Jesse Quale, Matt Welch
# SCHOOL:      Arizona State University
# CLASS:       CSE434: Introduction to Networks
# INSTRUCTOR:  Dr. Violet Syrotiuk
# SECTION:     71101
# TERM:        Fall 2013

simple-sockets
==============

Simple sockets client/server for CSE434: Networks

To run, test, or play with the simple-sockets program complete the following:

The program will require that you have two terminals open either both on the same machine or one on the current machine and another on a different machine connected to the same network. Once you have two terminal windows open navigate to the directory where the “Team-9.zip” files were extracted in both terminal windows.

Pick a terminal window and build the program by typing “make” (without quotes) at the command prompt and press enter. It does not matter what terminal you choose just pick one and it will build the program that will be used on both terminals.

Now delegate one of your open terminal windows as the client and the other as the server. The commands will differ depending on whether the terminal is acting as the client or the server.

In the server terminal window type “./UDPServer 65432” (without quotes) at the command prompt and press enter. A message will appear at the command prompt with an IP address followed by a 5-digit number. These numbers are needed for the next step so write them down if you can not see the server terminal window from the client terminal window. 

In the client terminal window type “./UDPClient” (without quotes) and press the space bar then enter the IP address from the server window and press the space bar then enter the 5-digit number that follows the IP address from the server window.  For example if the server produced the message 
“server is running on 253.234.21.5 12345” then you would type in the client window:
./UDPClient 253.234.21.5 12345 

Simple-sockets is now running and will produce output to the terminal describing its operation.

>>>>>>>>>>>>>>>>>>>>>> Some advanced options are below <<<<<<<<<<<<<<<<<<<<<<<<<

In the server window to automatically build and start server on port 65432 type
“make run” (without quotes) 

To automatically start multiple clients: in the client window type “./launch_multiple_clients.sh” (without quotes)  ONLY AFTER you first edit the launch_multiple_clients.sh file as follows. 
In your favorite text editor open  launch_multiple_clients.sh and change the SERVER_IP value to the IP address of the server (the IP address displayed in the message in the server terminal window)
The default operation of launch_multiple_clients.sh is to assume the server is running on general 1 at 129.10.241:65432
It will launch 5 client processes and send them to the background, while redirecting their output to individual files named output_client_N.tmp, where N is their client number between 1 and 5

>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Design Methodology of the client and server <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    The client and server in this program communicate with connectionless UDP sockets.  The implementation 
of these sockets is influenced by the UDP Echo Server program demonstrated to the students.  
    The server was designed with C++ data structures in mind, storing the client data in a <map>, where the key 
is the string "<IP address>_<Incarnation Number>_<Client Number>" and the value type was a vector of structs, 
each containing the request numbers and current client string held by the server.  When the client sends its 
first request, the server creates a key then looks up the client in its table.  If no entry is found, the 
server creates a new client instance in the table.  If the client key already exists in the table, the server
looks up the client data and compares the current request to the last request serviced.  If the request number 
of the new record is greater than that of the last record, the new data is stored in the table and an ACK is
sent to the client containing the current string held by the server.  If the request number of the new request
is equal to the most recent request stored by the server, the server ACKS with the stored string to the 
client.  If the new request number is older than the last request stored by the server, the server does nothing.
Each request is a standardized structure provided to us.  

    The server may fail in two ways, each with a probability of 10%.  The first failure mode has the server 
store the request data appropriately, but not send an ACK to the client.  This should cause the client to 
timeout and resend the request.  The second failure mode of the server does not store the data or send any ACK
to the client, which will also cause the client to resend the request.  

    The client also stores its current string for comparison to the server ACK.  When the server responds, 
the client compares the returned string to its string to determine if it should resend the request.  In response
to server failures, the client will attempt to resend its request to the server up to 20 times before giving 
up and aborting the program.  Upon a successful send and comparison to the server ACK, the client will proceed
to the next request iteration.  

    The client has only one failure mode where it may, after a random number of requests, begin failing.  
At this point, each request has a 50% chance of failing, which causes the client to update an incarnation 
number stored in the file inc.txt.  The client then proceeds to send the request to the server, which will 
result in the server identifyig it as a new client key and creating a new entry in its table.  At each request 
iteration, the client also checks to see if another client on the same host has updated the incarnation number 
then updates its incarnation number accordingly. The incarnation number file is protected by a lock that
prevents reads by other clients whila a client is updating the incarnation number stored within.   

