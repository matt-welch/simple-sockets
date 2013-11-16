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
 **************************************************************************** */

#include "UDPServer.hpp" // server header file 

#include <sstream>
using std::stringstream;

#include <iostream>
using std::cout;
using std::endl;
using std::cin;

#define DEBUG 1
#define STORE_CLIENT_DATA 1
#define STORE_WITH_FUNCTION 1

// function to reply to the client with its current string
// TODO all input arguments can be const
void ackToClient(const int& sock, const char* clientString, const struct sockaddr_in& echoClntAddr ){
    char localString[strLen+1];
    strcpy(localString, clientString);
    int bytesSent = sendto( sock, localString, strLen+1, 0, 
                (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr) );
    
#ifdef DEBUG
    cout << "Server returned new client string <" << localString << "> to client at " 
        << inet_ntoa(echoClntAddr.sin_addr) << " (" << bytesSent << " bytes sent)" <<endl;
#endif // DEBUG
    /* Send current client string back to the client */
    if ( bytesSent != (strLen+1) )
        DieWithError("sendto() sent a different number of bytes than expected");
}

// function to handle the client request by looking up and/or storing in the
// client table
// TODO this function should not return the newString, but should take it as
// an input argument reference like the table and request
int handleClientData( client_table_t& clientTable, request_t& clientRequest, string& newString){
    client_data_t clientVector;
    stringstream clientKey;
	char clientString[strLen+1] = "     ";			 /* 5-element string belonging to the client */
    int returnCode = -1;
    int numRecords = 0;
    int requestNum = 0;

    // assemble the client key string from the components of the clients request
    clientKey.str(""); // clear the key 
    // assemble the key
    clientKey << clientRequest.client_ip << "_" << clientRequest.client << "_"
        << clientRequest.inc;
#ifdef DEBUG
    cout << "ClientKey = " << clientKey.str() << endl;
#endif
    requestNum = clientRequest.req; 

    /* determine if the clientTable already has data from this client */
    client_table_t::iterator table_it= clientTable.find(clientKey.str());

    if(table_it == clientTable.end()) {// no entry found, create new
#ifdef DEBUG
        cout << "Client key <" << clientKey.str() << 
            "> not found in the table, creating new entry:" << 
            "< " << clientKey.str() << ", '" << clientRequest.c << "' >" << endl;
#endif
        // copy in the new string (one character)
        newString = clientRequest.c;
        returnCode = 1;
    }else{// client data found, get it from the table and add the new string
        // get the client data from the table
        clientVector = table_it->second; //clientTable[clientKey.str()];
        numRecords = clientVector.size();

        if ( requestNum > numRecords ) {
            // R > r  // assumed to be the common case so it's first
            /* new request store the record in the table, send the response*/
#ifdef DEBUG
        printf("Server:: handleClientData():: %d > %d, storing response\n", requestNum, numRecords ); 
#endif
            // copy the last string from the table to the client String
            strcpy(clientString, clientVector.back().c_str());
            // modify the string stored with the client
            updateClientString(clientString, clientRequest.c, strLen);
            // create a new string to add to the client data
            newString = clientString;
            returnCode = 1;

        }else if( requestNum == numRecords ){
            // R == r
            // send the stored repsonse to the client
#ifdef DEBUG
        printf("Server:: handleClientData():: %d == %d, sending stored response\n", requestNum, numRecords ); 
#endif
            newString = clientVector.back();
            returnCode = 0;
        }else{
            // R < r
#ifdef DEBUG
        printf("Server:: handleClientData():: %d < %d, old request, don't store\n", requestNum, numRecords ); 
#endif
            // the request is old, ignore it and do not send the response
            returnCode = -1;
        }
    }
    if(returnCode > 0){
        // push the updated string onto the client data
        clientVector.push_back(newString);
        // update the client table with new string
        clientTable[clientKey.str()] = clientVector;
    }
   
    // return code based on old/recent/new data  {-1, 0, 1} 
    return returnCode; 
}

int main(int argc, char *argv[])
{
    int sock;                        /* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen;         /* Length of incoming message */
    unsigned short echoServPort;     /* Server port */
	int recvMsgSize;                 /* Size of received message */
	char clientString[strLen+1] = "     ";	 /* 5-element string belonging to the client */
    stringstream clientKey;
    client_data_t clientVector;
    string newString;
	int fp, failureProbability = 0;
    int returnCode = -1;

	/* random seed */
    srand(time(NULL));

	/* variables to contain data sent from client and the table of client data */
	request_t clientRequest;
	client_table_t clientTable;
    if (argc != 2)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }
	
    echoServPort = atoi(argv[1]);  /* First arg:  local port */

    /* display the server's IP address  */
    cout << "Server is running at " << getSocketIP() << ":" << echoServPort << endl;

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
        // randomly calculate the failureProbability for each iteration 
        // past the failure point (0 to 10) 
        failureProbability = rand() % 10;

        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(echoClntAddr);

        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, &clientRequest, sizeof(request_t), 
                        0,(struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");
#ifdef DEBUG
		printf("Server:: received client data::\n");
        printRequestStructure(clientRequest);
#endif
        //fail 1 do nothing
        //fail 2 add data to table but no send response
        fp = failureProbability;
        cout << "Server: fp = " << fp << endl;
        if(fp >= 2)
        {//do right thing
#ifdef DEBUG
            cout << "Server: Normal operation: handling client data and sending ack to client" << endl;
            printf( "Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr) );
#endif
            returnCode =  handleClientData(clientTable, clientRequest, newString);
            strcpy(clientString, newString.c_str() );
            if (returnCode < 0) {
                // clientRequest.req < last_stored_req
                // data is old, do not re-send the ack
                // do nothing
            }else if (returnCode == 0) {
                // clientRequest.req == last_stored_req
                // send the stored response to the client
                // send acknowledgement to the client
#ifdef DEBUG
                cout << "Server: about to ack to client" << endl;
#endif
                ackToClient(sock, clientString, echoClntAddr );
            }else{ // TODO OPTIMIZATION: this is not necessary and code0,1 can be handles in same case
                // clientRequest.req > last_stored_request
                // data is new, store the response and send the ack
                // send acknowledgement to the client
#ifdef DEBUG
                cout << "Server: about to ack to client" << endl;
#endif
                ackToClient(sock, clientString, echoClntAddr );
            }

        }
        else if(fp == 1)
        {//add data to table but no response		
#ifdef DEBUG
            cout << "Server: Failure Mode 1: handling client data DON'T ack to client" << endl;
#endif
            returnCode =  handleClientData(clientTable, clientRequest, newString);
            strcpy(clientString, newString.c_str() );
        }
        else if(fp == 0)
        {//do nothing
#ifdef DEBUG
            cout << "Server: Failure Mode 2: do nothing " << endl;
#endif
            continue;
        }
			
#ifdef DEBUG
        // new line to space out requests
        cout << endl;
#endif // DEBUG
    }
    /* NOT REACHED */
}
