/* **************************************************************************
 * FILENAME:    UDPServer.cpp
 * NAME:        UDP Simple Sockets server program
 * AUTHORS:     Jesse Quale, Matt Welch
 * SCHOOL:      Arizona Statte University
 * CLASS:       CSE434: Introduction to Networks
 * INSTRUCTOR:  Dr. Violet Syrotiuk
 * SECTION:     
 * TERM:        Fall 2013
 * DESCRIPTION: 
 *      This program is the server-side of a simple UDP sockets based
 * client-server.  The client is to send a structure to the server containing
 * information identifying the client, the particular request number, and
 * a character [a-z] that reprsents the payload of the exchange.  The server
 * is to keep track of the last five characters of the exchange and a table of
 * the client requests so that it can resend responses and service requests
 * based on simulated failure modes of both the client and the server.  The
 * algorithm of this eschange is more fully described in the README.MD.  
 * */

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "UDPServer.hpp" // server header file 

#include <sstream>
using std::stringstream;

#include <iostream>
using std::cout;
using std::endl;
using std::cin;

#define DEBUG 1

void DieWithError(const char *errorMessage) /* External error handling function */
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sock;                        /* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen;         /* Length of incoming message */
//    char echoBuffer[ECHOMAX];        /* Buffer for echo string */
    unsigned short echoServPort;     /* Server port */
	int recvMsgSize;                 /* Size of received message */
	char clientString[strLen+1] = "     ";			 /* 5-element string belonging to the client */
    stringstream keyStream;

	/* variables to contain data sent from client and the table of client data */
	request_t clientRequest;
	client_table_t clientTable;

    if (argc != 2)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  /* First arg:  local port */

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");
  
    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(echoClntAddr);

        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, &clientRequest, sizeof(request_t), 
						0,(struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");

#ifdef DEBUG
        printf( "Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr) );

		printf("Server:: received client data::\n");
		printf("clientRequest.client_ip(char*)\t= %s\n", clientRequest.client_ip);
		printf("clientRequest.inc (int)\t\t= %d\n", clientRequest.inc);
		printf("clientRequest.client (int)\t= %d\n", clientRequest.client);
		printf("clientRequest.req (int\t)\t= %d\n", clientRequest.req);
		printf("clientRequest.c (char)\t\t= %c\n", clientRequest.c);
#endif
		// TODO assemble the clientKey string from the components of the
		// clients request
        keyStream.str(""); // clear the key 
        // assemble the key
        keyStream << clientRequest.client_ip << "_" << clientRequest.client << "_"
            << clientRequest.inc;
#ifdef DEBUG
        cout << "ClientKey = " << keyStream.str() << endl;
#endif
		//string clientKey = keyStream.str();;
		/* determine if the clientTable already has data from this client */
		client_table_t::iterator table_it= clientTable.find(keyStream.str());

		//TODO if the request is not already in the table, add it
        if(table_it == clientTable.end()) {
//            client_data_t clientVector;
//            clientVector[0] = clientRequest;
//            clientTable.insert(keyStream.str(), clientVector);
        }else{
            // lookup the client data in the table and add the new request
        }
        

		// modify the string stored with the client
		updateClientString(clientString, clientRequest.c, strLen);
#ifdef DEBUG
		printf("New client string is %s\n", clientString);
#endif
		/* copy client request data into the clientTable */

#ifdef ACK_TO_CLIENT
        /* Send current client string back to the client */
        if ( sendto(sock, clientString, sizeof(clientString), 0, 
             (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(clientString) )
            DieWithError("sendto() sent a different number of bytes than expected");
#ifdef DEBUG
        cout << "Server returned client string <" << clientString << "> to client at " << inet_ntoa(echoClntAddr.sin_addr) << endl;
#endif
#endif // ACK_TO_CLIENT
#ifdef DEBUG
        // new line to space out requests
        cout << endl;
#endif // DEBUG
    }
    /* NOT REACHED */
}
