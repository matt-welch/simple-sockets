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
#define SHOWERRORS 1
//#define DEBUG 1
#define ECHOMAX 255     /* Longest string to echo */
//-----------------------------------------------------

// lock for accessing the incarnation file
pthread_mutex_t g_lock_incarnationFile;

void DieWithError(const char *errorMessage) /* External error handling function */
{
    perror(errorMessage);
    exit(1);
}

int getValueFromFile(string filename){
    // open the file as a filestream
    // lock file access here
    ifstream infile( filename.c_str() );
    string inString;
    int int_incarNumber=0;

// open the file for read-only access
#ifdef DEBUG
    printf("getValueFromFile():: About to open file \"%s\"\n",filename.c_str());
#endif
    if( infile.fail() ) {
        cout << "No file <" << filename << "> found; creating file with incarnation number = 0" 
            << endl;
        // create the file because it does not yet exist and fill it with a 0
        ofstream outFile(filename.c_str() );
        outFile << (int_incarNumber ) << endl;
        outFile.close();
    }else{
        getline(infile, inString);
#ifdef DEBUG
        cout << "getValueFromFile():: incarnation number found = <" << inString << ">" << endl;
#endif
        if(inString.length() > 0)
            int_incarNumber = atoi( inString.c_str() );
    }
    return int_incarNumber;
}

// function to check on the incarnation number.  If it is different than the
// currently held number, call updateIncarNum() and return the updated value.
int checkIncarNum(void){
    string filename = "inc.txt";
    string str_incarNumber;
    int int_incarNumber=0;
    int_incarNumber = getValueFromFile(filename);

// return file value for inc always
    return(int_incarNumber);
}

// the number 
// function unlocks file containing last incarnation number and gets a new
int updateIncarNum(void){
    string filename = "inc.txt";
    string str_incarNumber;
    int int_incarNumber=0;

    // lock file access, perform read & write new incarnation number to file
    //pthread_mutex_lock(&g_lock_incarnationFile);
    // open the file as a filestream
    // since this is the failure mode, increment the incarnation number 
    int_incarNumber = getValueFromFile(filename) + 1;
#ifdef UNUSED
    ifstream infile( filename.c_str() );
    string inString;

    if( infile.fail() ) {
#if SHOWERRORS
        cout << "No file <" << filename << "> found; creating file with incarnation number = 0" 
            << endl;
#endif // SHOWERRORS
    }else{
        getline(infile, inString);
#ifdef DEBUG
        cout << "Incarnation Number = <" << inString << ">" << endl;
#endif
        if(inString.length() > 0)
            int_incarNumber = atoi( inString.c_str() );

    }
#endif //UNUSED     
    // update the incarnation number to the new value
    ofstream outFile(filename.c_str() );
    outFile << (int_incarNumber ) << endl;
    outFile.close();

    //pthread_mutex_unlock(&g_lock_incarnationFile);
    return(int_incarNumber);
}

int main(int argc, char* argv[])//char  *argv[]
{
    int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort;     /* Echo server port */
    char *servIP;                    /* IP address of server */
    const int MAX_REQUESTS = 20;     /* maximum number of client requests */
	int requestNum = 0;              /* iterator variable for requests loop */
    int failurePoint;                /* the iteration at which failures may begin */
    float failureProbability = 0.0;  /* probability of failure past failurePoint */
#ifdef RCV_FROM_SERVER  
    unsigned int fromSize;           /* In-out of address size for recvfrom() */
    struct sockaddr_in fromAddr;     /* Source address of echo */
    char *echoString;                /* String to send to echo server */
    char echoBuffer[ECHOMAX+1];      /* Buffer for receiving echoed string */
    int echoStringLen;               /* Length of string to echo */
    int respStringLen;               /* Length of received response */
#endif // RCV_FROM_SERVER

    int clientNum;                   /* client number, passed in as a command line argument */
	request_t clientRequest; //new reques

	/* random seed */
	srand(time(NULL));

    // parse command line arguments
    if ((argc < 3) || (argc > 4))    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Server IP> <Server Port> <ClientNumber>\n", argv[0]);
        exit(1);
    }
    servIP = argv[1];           /* First arg: server IP address (dotted quad) */
    echoServPort = atoi(argv[2]);  /* Use given port */
    //echoServPort = 65432;  /* experimental port range port as default */
    clientNum = atoi(argv[3]); 

    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
  	memset(&echoServAddr, 0, sizeof(echoServAddr));    /* Zero out structure */
      echoServAddr.sin_family = AF_INET;                 /* Internet addr family */
      echoServAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
      echoServAddr.sin_port   = htons(echoServPort);     /* Server port */

	//------- get IP ----------- // should be a function
	//*** maybe use getpeername to get info like IP from socket
	//I will check into it soon : http://support.sas.com/documentation/onlinedoc/sasc/doc700/html/lr2/zeername.htm
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
    failurePoint = rand() % (MAX_REQUESTS) ; // the iteration at which failures wil begin with a probability of 50%
    cout << "Client number " << clientNum << " will fail at iteration " << failurePoint+1 << endl;
 
	/* build struct */
	//clientRequest.client_ip = "129.219.102.8\0\0"
	

	clientRequest.client = clientNum;
	for(requestNum = 0; requestNum < MAX_REQUESTS ; ++requestNum)
	{
		clientRequest.req = requestNum+1;
		// randomize the char sent to the server
		clientRequest.c   = (char) ( 97 + (rand() % 26) ); // 97 == ascii "a"



		// simulate failure modes of client
        // update the incarnation number if past the point of failure
        // get incarnation number by unlocking file, incrementing, and
        // closing file
        if((requestNum) >= failurePoint){
            // randomly calculate the failureProbability for each iteration 
            // past the failure point (0 to 1) 
            failureProbability = (float) rand() / (float) RAND_MAX;
#ifdef DEBUG
            cout << "Failure Probability = " << failureProbability; 
#endif
            if(failureProbability > 0.5){   
                // failure occured, increment the current incarnation
#ifdef DEBUG
                cout << " --> FAILURE!! " << endl;
#endif
                clientRequest.inc = updateIncarNum();
            }else{
                // failure did not happen, just get the current incarnation
                // number
#ifdef DEBUG
                cout << " --> SUCCESS!! " << endl;
#endif
                clientRequest.inc = checkIncarNum();
            }
        }else{
            // no failures yet, just check that nobody else has updated it
            clientRequest.inc = checkIncarNum();
        }

        
#ifdef DEBUG
		printf("Client:: sending client data::\n");
		printf("clientRequest.client_ip(char*)\t= %s\n", clientRequest.client_ip);
		printf("clientRequest.inc (int)\t\t= %d\n", clientRequest.inc);
		printf("clientRequest.client (int)\t= %d\n", clientRequest.client);
		printf("clientRequest.req (int)\t\t= %d\n", clientRequest.req);
		printf("clientRequest.c (char)\t\t= %c\n\n", clientRequest.c);
#endif
		/* Send the string to the server */
		if (sendto(sock, &clientRequest, sizeof(clientRequest), 0, (struct sockaddr *)
					&echoServAddr, sizeof(echoServAddr)) != sizeof(clientRequest))
			DieWithError("sendto() sent a different number of bytes than expected");
#ifdef DEBUG
        cout << endl;
#endif
        // need to receive a response form the server to verify the packet was
        // received??
	}//end for




#ifdef RCV_FROM_SERVER
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
#endif // RCV_FROM_SERVER

    close(sock);
    cout << "Client Program terminated. " << endl;
    exit(0);
}//end main
