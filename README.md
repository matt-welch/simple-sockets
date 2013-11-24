simple-sockets
==============
Filename:    README.md
Name:        readme for UDP Simple Sockets client and server programs
Authors:     Jesse Quale, Matt Welch
School:      Arizona State University
Class:       CSE434: Introduction to Networks
Instructor:  Dr. Violet Syrotiuk
Section:     71101
Term:        Fall 2013
Description: Simple sockets client/server for CSE434: Networks

Instructions
------------
To run, test, or play with the simple-sockets program complete the following:

The program will require that you have two terminals open either both on the same machine
or one on the current machine and another on a different machine connected to the same
network. Once you have two terminal windows open navigate to the directory where the
Team-9.zip files were extracted in both terminal windows.

Build the program by typing
 
   make

at the command prompt in one terminal if on the same host and both terminals if on 
different hosts and press enter. 
Now delegate one of your open terminal windows as the client and the other as the server.
The commands will differ depending on whether the terminal is acting as the client or the server.

The general form of the server command line operation is 

    ./UDPServer <Port Number>

Select a port above 1024 (we like 65432) and, in the server terminal window type

    ./UDPServer 65432

at the command prompt and press enter. A message will appear at the command prompt 
indicating the IP address and port used by the server in the form <IP address>:<port> e.g.

    ./UDPServer 65432
    Server is running at 129.219.10.241:65432

These numbers are needed for the next step so write them down if you can not see the server
terminal window from the client terminal window.

The general form of the client command is: 

    ./UDPClient <server IP address> <server port number> [client number] [sleep time]

where the <> brackets indicate required arguments and the [] brackets indicate optional 
arguments with defaults of clientNumber 0 and sleep time 500,000 microseconds, respectively.

In the client terminal window, begin the client operation with a command like the following:

    ./UDPClient 129.219.10.241 65432 42 

which would launch the client to connect to the server at 129.219.10.241 on port 65432 
with client number 42.  

Simple-sockets is now running and will produce output to the terminal describing its operation.

Some advanced options
---------------------
In the server window to automatically build and start a server on port 65432 type 

    make run

which will also pipe the server output through the tee command into the file output_server.tmp

There is also a bash script called launch_multiple_clients.sh that assumes that the server is
running on the ASU general1 server at 129.219.10.241 on port 65432.  If the server is running 
elsewhere, first edit the script to match the server IP (SERVER_IP) and port numbers (PORTNUM).  
To automatically start 5 clients in the client window type 

    ./launch_multiple_clients.sh

ONLY AFTER you first edit the launch_multiple_clients.sh file as described above.  
It will launch 5 client processes and send them to the background, while redirecting their output 
to files named output_client_N.tmp, where N is their client number.

Design Methodology of the client and server
-------------------------------------------

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

