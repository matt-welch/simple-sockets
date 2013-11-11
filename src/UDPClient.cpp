/* **************************************************************************
 * FILENAME:    UDPClient.cpp
 * NAME:        UDP Simple Sockets client program
 * AUTHORS:     Jesse Quale, Matt Welch
 * SCHOOL:      Arizona Statte University
 * CLASS:       CSE434: Introduction to Networks
 * INSTRUCTOR:  Dr. Violet Syrotiuk
 * SECTION:     
 * TERM:        Fall 2013
 * DESCRIPTION: 
 *      This program is the client-side of a simple UDP sockets based
 * client-server.  The client is to send a structure to the server containing
 * information identifying the client, the particular request number, and
 * a character [a-z] that reprsents the payload of the exchange.  The server
 * is to keep track of the last five characters of the exchange and a table of
 * the client requests so that it can resend responses and service requests
 * based on simulated failure modes of both the client and the server.  The
 * algorithm of this eschange is more fully described in the README.MD.  
 * */

// TODO replace c-libraries with C++ libraries
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "request.hpp"  // header
#include <time.h>

//for getting ip info
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <iostream>
using std::cout;
using std::endl;
using std::cin;
using std::ostream;
using std::istream;
using std::getline;

#include <fstream>
using namespace std;

#include <string>
using std::string;

#include <pthread.h>  /* for mutex lock on file access */

#define USEFSTREAM
#define DEBUG 1
#define ECHOMAX 255     /* Longest string to echo */
//-----------------------------------------------------

// lock for accessing the incarnation file
pthread_mutex_t g_lock_incarnationFile;

void DieWithError(const char *errorMessage) /* External error handling function */
{
    perror(errorMessage);
    exit(1);
}

// function unlocks file containing last incarnation number and gets a new
// number for the client after a failure
int getIncarnationNum(void){
    string filename = "incarnum";
    string str_incarNumber;
    int int_incarNumber=0;

#ifdef DEBUG
    printf("getIncarnationNum::About to open file \"%s\"\n",filename.c_str());
#endif
    // lock file access, perform read & write new incarnation number to file
    pthread_mutex_lock(&g_lock_incarnationFile);
        // open the file as a filestream
            ifstream infile( filename.c_str() );
            string inString;

            if( infile.fail() ) {
                cout << "No file <" << filename << "> found; creating file with incarnation number = 0" 
                    << endl;
            }else{
                getline(infile, inString);
#ifdef DEBUG
                cout << "Incarnation Number = <" << inString << ">" << endl;
#endif
                if(inString.length() > 0)
                    int_incarNumber = atoi( inString.c_str() );
                
            }
#ifdef DEBUG
            cout << "TEST:: <" << inString << "> (" << int_incarNumber << ")" <<  endl;
            cout << "\tIncarnation Number = " << int_incarNumber << endl;
#endif
            ofstream outFile(filename.c_str() );
            outFile << (int_incarNumber + 1) << endl;
            outFile.close();

    pthread_mutex_unlock(&g_lock_incarnationFile);
    return(int_incarNumber);
}

int main(int argc, char* argv[])//char  *argv[]
{
    int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    struct sockaddr_in fromAddr;     /* Source address of echo */
    unsigned short echoServPort;     /* Echo server port */
    unsigned int fromSize;           /* In-out of address size for recvfrom() */
    char *servIP;                    /* IP address of server */
    char *echoString;                /* String to send to echo server */
    char echoBuffer[ECHOMAX+1];      /* Buffer for receiving echoed string */
    int echoStringLen;               /* Length of string to echo */
    int respStringLen;               /* Length of received response */

	request_t clientRequest; //new reques

	/* random seed */
	srand(time(NULL));

    if ((argc < 3) || (argc > 4))    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];           /* First arg: server IP address (dotted quad) */
    echoString = argv[2];       /* Second arg: string to echo */

    if ((echoStringLen = strlen(echoString)) > ECHOMAX)  /* Check input length */
        DieWithError("Echo word too long");

    if (argc == 4)
        echoServPort = atoi(argv[3]);  /* Use given port, if any */
    else
        echoServPort = 7;  /* 7 is the well-known port for the echo service */

    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
  	memset(&echoServAddr, 0, sizeof(echoServAddr));    /* Zero out structure */
      echoServAddr.sin_family = AF_INET;                 /* Internet addr family */
      echoServAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
      echoServAddr.sin_port   = htons(echoServPort);     /* Server port */

	//------- get IP ----------- // should be a function
	int socketDescriptor;//
	struct ifreq interface;
	
	socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
	
	//get IPv4 address ################################################
	interface.ifr_addr.sa_family = AF_INET;
	
	//get address attached to eth0
	strncpy(interface.ifr_name, "p4p1", IFNAMSIZ-1);
	
	ioctl(socketDescriptor, SIOCGIFADDR, &interface);
	
	close(socketDescriptor);
	
	//---- we have IP now ????????????????????????????????????????????????????????
	
	strcpy(clientRequest.client_ip, inet_ntoa(((struct sockaddr_in *) &interface.ifr_addr)->sin_addr));
	
	//print our IP address
	printf("IP address = %s\n", clientRequest.client_ip);
    cout << endl;
	int requestNum = 0;
	/* build struct */
	//clientRequest.client_ip = "129.219.102.8\0\0"
	

	clientRequest.client = 42;
	for(requestNum = 0; requestNum < 5; ++requestNum)
	{
		clientRequest.req = requestNum + 1;
		//TODO randomize the char sent to the server
		clientRequest.c   = (char) 97 + requestNum; // 97 == ascii "a"

		//TODO simulate failure modes of client

        // get incarnation number by unlocking file, incrementing, and
        // closing file
        clientRequest.inc = getIncarnationNum();
#ifdef DEBUG
        printf("clientRequest.inc = %d \n", clientRequest.inc);
        printf("Client :: sending request #%d\n", clientRequest.req);
#endif
		/* Send the string to the server */
		if (sendto(sock, &clientRequest, sizeof(clientRequest), 0, (struct sockaddr *)
					&echoServAddr, sizeof(echoServAddr)) != sizeof(clientRequest))
			DieWithError("sendto() sent a different number of bytes than expected");
        cout << endl;
	}//end for





    /* Recv a response */
    fromSize = sizeof(fromAddr);
    if ((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0,
         (struct sockaddr *) &fromAddr, &fromSize)) != echoStringLen)
        DieWithError("recvfrom() failed");

    if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
    {
        fprintf(stderr,"Error: received a packet from unknown source.\n");
        exit(1);
    }

    /* null-terminate the received data */
    echoBuffer[respStringLen] = '\0';
    printf("Received: %s\n", echoBuffer);    /* Print the echoed arg */

    close(sock);
    exit(0);
    return (0);
}//end main
