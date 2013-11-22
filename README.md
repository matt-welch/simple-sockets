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
./UDPClient 253.234.21 12345 

Simple-sockets is now running.

>>>>>>>>>>>>>>>>>>>>>> Some advanced options are below <<<<<<<<<<<<<<<<<<<<<<<<<

In the server window to automatically build and start server on port 65432 type
“make run” (without quotes) 

To automatically start multiple clients: in the client window type “./launch_multiple_clients.sh” (without quotes)  ONLY AFTER you first edit the launch_multiple_clients.sh file as follows. 
In your favorite text editor open  launch_multiple_clients.sh and change the SERVER_IP value to the IP address of the server (the IP address displayed in the message in the server terminal window)


 
