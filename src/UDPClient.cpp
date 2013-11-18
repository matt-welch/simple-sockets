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
#include "request.hpp"  // header
#include <time.h>
#include <unistd.h> // for usleep TODO nanosleep is better

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

#define SHOWERRORS 1
#if ACK_TO_CLIENT
    #define RECEIVE_FROM_SERVER 1
#endif

//#define DEBUG 1
//-----------------------------------------------------
void receiveAckFromServer( int& sock, const struct sockaddr_in& echoServAddr ){
    unsigned int fromSize;           /* In-out of address size for recvfrom() */
    int respStringLen;               /* Length of received response */
    struct sockaddr_in fromAddr;     /* Source address of echo */
    char echoBuffer[ECHOMAX+1];      /* Buffer for receiving echoed string */

    /* Recv a response */
    fromSize = sizeof(fromAddr);
    respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *) &fromAddr, &fromSize);
#ifdef DEBUG
    cout << "Bytes Received = " << respStringLen << ", Bytes expected = " << strLen+1 << endl;
#endif
    if ( respStringLen != strLen+1 )
        DieWithError("recvfrom() failed");

    if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
    {
        fprintf(stderr,"Error: received a packet from unknown source.\n");
        exit(1);
    }

    /* null-terminate the received data */
    echoBuffer[respStringLen] = '\0';
    printf("String Received from server:\t\t%s\n", echoBuffer);    /* Print the echoed arg */


}

int getValueFromFile(string filename){
    // open the file as a filestream
    // lock file access here
    ifstream infile( filename.c_str() );
    string inString;
    int int_incarNumber=0;

// open the file for read-only access
#ifdef VERBOSE
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
#ifdef VERBOSE
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
    const float chanceOfFailure = 0.5; 
#ifdef RECEIVE_FROM_SERVER  
    unsigned int fromSize;           /* In-out of address size for recvfrom() */
    struct sockaddr_in fromAddr;     /* Source address of echo */
	char clientString[strLen+1] = "     "; /* 5-element string belonging to the client */
    char echoBuffer[ECHOMAX+1];      /* Buffer for receiving echoed string */
    //int echoStringLen = strLen+1;               /* Length of string to echo */
    int respStringLen;               /* Length of received response */
#endif // RECEIVE_FROM_SERVER
    unsigned int sleepTime = 500000; // sleep time for client in microseconds (us)
    int clientNum = 0;                   /* client number, passed in as a command line argument */
	request_t clientRequest; //new reques

	/* random seed */
	srand(time(NULL));

#ifdef VERBOSE
    cout << "Client Argument list: " << argc << " arguments supplied" << endl;
    for (int i = 0; i < argc; i++) {
        cout << "Arg[" << i << "] = <" << argv[i] << ">" <<  endl;
    }
#endif

    // parse command line arguments
    if ( (argc < 3) || (argc > 5) )    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Server IP> <Server Port> [ClientNumber] [sleeptime(us)]\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];           /* First arg: server IP address (dotted quad) */
    echoServPort = atoi(argv[2]);  /* Use given port */
    //echoServPort = 65432;  /* experimental port range port as default */
    if ( argc > 3) {
        // client number has been supplied as the third command line argument
        clientNum = atoi(argv[3]); 
#ifdef DEBUG
        printf("clientNum = %d\n", clientNum);
#endif
        if (argc > 4) {
            // sleep time has been supplied
            sleepTime = atoi(argv[4]); 
#ifdef DEBUG
            printf("sleep time = %d\n", sleepTime);
#endif
        }
    }else{
        cout << "No client number supplied, using clientNum=0" << endl;
    }


    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
  	memset(&echoServAddr, 0, sizeof(echoServAddr));    /* Zero out structure */
      echoServAddr.sin_family = AF_INET;                 /* Internet addr family */
      echoServAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
      echoServAddr.sin_port   = htons(echoServPort);     /* Server port */

#ifdef UNUSED

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
#endif    	


    strcpy(clientRequest.client_ip, getSocketIP() ); 

	//print our IP address
	printf("Client IP address = %s\n", clientRequest.client_ip);
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
        // TODO should the client send data if it fails?
        if((requestNum) >= failurePoint){
            // randomly calculate the failureProbability for each iteration 
            // past the failure point (0 to 1) 
            failureProbability = (float) rand() / (float) RAND_MAX;
#ifdef DEBUG
            cout << "Failure Probability = " << failureProbability; 
#endif
            if(failureProbability < chanceOfFailure){   
#ifdef DEBUG
                cout << " --> FAILURE!! " << endl;
#endif
                // failure occured, increment the current incarnation
                clientRequest.inc = updateIncarNum();
                // reset the client string to empty 5 char
                strcpy(clientString, "     ");
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

        do{ 
#ifdef DEBUG
            printf("Client:: sending client data::\n");
            printRequestStructure(clientRequest);
#endif
            /* Send the string to the server */
            if (sendto(sock, &clientRequest, sizeof(clientRequest), 0, (struct sockaddr *)
                        &echoServAddr, sizeof(echoServAddr)) != sizeof(clientRequest))
                DieWithError("sendto() sent a different number of bytes than expected");
            // modify the string stored with the client
            updateClientString(clientString, clientRequest.c, strLen);
#ifdef DEBUG  //"String Received from server: "
            cout << "Client string (stored by client):\t" << clientString <<  endl;
#endif

            // need to receive a response form the server to verify the packet was
            // received??
#ifdef RECEIVE_FROM_SERVER
            /* Recv a response */
            /* TODO make this a function */
            fromSize = sizeof(fromAddr);
            respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0,
                        (struct sockaddr *) &fromAddr, &fromSize);
#ifdef DEBUG
            cout << "Bytes Received = " << respStringLen << ", Bytes expected = " << sizeof(clientString) << endl;
#endif
            if ( respStringLen != sizeof(clientString) )
                DieWithError("recvfrom() failed");

            if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
            {
                fprintf(stderr,"Error: received a packet from unknown source.\n");
                exit(1);
            }

            /* null-terminate the received data */
            echoBuffer[respStringLen] = '\0';
            printf("String Received from server:\t\t%s\n", echoBuffer);    /* Print the echoed arg */
#endif // RECEIVE_FROM_SERVER

            // put the client to sleep for a bit - should be an input argument
            // in seconds or milliseconds would be better
#ifdef DEBUG
            cout << "Client: sleeping for "<< sleepTime << " us...." << endl;
#endif           
            usleep(sleepTime);
            // TODO need to compare string of server ack to the string held by
            // the client to determine if the client needs to resend data
            // strcmp( echoBuffer, clientString)
            // if same, move on
            // else if server is older by one character, resend
            // else if server is newer by one character, move on
        }while (0);

#ifdef DEBUG
        cout << endl;
#endif
	}//end for

    close(sock);
    cout << "Client Program terminated. " << endl;
    exit(0);
}//end main
