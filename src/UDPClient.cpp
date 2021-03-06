/* **************************************************************************
 * FILENAME:    UDPClient.cpp
 * NAME:        UDP Simple Sockets client program
 * AUTHORS:     Jesse Quale, Matt Welch
 * SCHOOL:      Arizona State University
 * CLASS:       CSE434: Introduction to Networks
 * INSTRUCTOR:  Dr. Violet Syrotiuk
 * SECTION:     71101
 * TERM:        Fall 2013
 * DESCRIPTION: 
 *      This program is the client-side of a simple UDP sockets based
 * client-server.  The client is to send a structure to the server containing
 * information identifying the client, the particular request number, and
 * a character [a-z] that reprsents the payload of the exchange.  The server
 * is to keep track of the last five characters of the exchange and a table of
 * the client requests so that it can resend responses and service requests
 * based on simulated failure modes of both the client and the server.  The
 * algorithm of this exchange is more fully described in the README.MD.  
 * */

#include "request.hpp"  // header
#include <time.h>
#include <unistd.h> 

//for getting ip info
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>
//for using fcntl()
#include <fcntl.h>
#include <iostream>
using std::cout;
using std::endl;
using std::cin;
using std::ostream;
using std::istream;
using std::getline;
 

#include <fstream>
using namespace std;

#if ACK_TO_CLIENT
    #define RECEIVE_FROM_SERVER 1
#endif

#define DEBUG 1

int getValueFromFile(string filename){
    // open the file as a filestream
    // lock file access here
    struct flock flockStruct;//this struct will describe what type of lock 
	int fd = 0;				 //we need and we will pass the address in fcntl() call
   /*build struct*/
	flockStruct.l_type   = F_WRLCK; //this is what type of lock in this case write-lock
 	flockStruct.l_whence = SEEK_SET; //start at beginning of file
	flockStruct.l_start  = 0; //offset for l_whence
	flockStruct.l_len    = 0; //length of of lock region; 0 = EOF
	flockStruct.l_pid    = getpid();      //get the process ID of process dealing with lock

	fd = open("filename", O_WRONLY); //open file in write only mode

	fcntl(fd, F_SETLKW, &flockStruct); //do the locking with fcntl


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

	/*unlock file*/
	flockStruct.l_type = F_UNLCK; //change struct field to unlock 
	fcntl(fd, F_SETLK, &flockStruct); //unlock with fcntl()

    return int_incarNumber;
}//end getValueFromFile

// function to check on the incarnation number. 
int checkIncarNum(void){
    string filename = "inc.txt";
    string str_incarNumber;
    int int_incarNumber=0;
    int_incarNumber = getValueFromFile(filename);

// return file value for inc always
    return(int_incarNumber);
}

// function unlocks file containing last incarnation number and gets a new one
int updateIncarNum(void){
    string filename = "inc.txt";
    string str_incarNumber;
    int int_incarNumber=0;

    // lock file access, perform read & write new incarnation number to file
    //pthread_mutex_lock(&g_lock_incarnationFile);
    // open the file as a filestream
    // since this is the failure mode, increment the incarnation number 
	struct flock flockStruct;//this struct will describe what type of lock 
	int fd = 0;				 //we need and we will pass the address in fcntl() call
	/*build struct*/
	flockStruct.l_type   = F_WRLCK; //this is what type of lock in this case write-lock
    flockStruct.l_whence = SEEK_SET; //start at beginning of file
	flockStruct.l_start  = 0; //offset for l_whence
	flockStruct.l_len    = 0; //length of of lock region; 0 = EOF
	flockStruct.l_pid    = getpid();      //get the process ID of process dealing with lock

	fd = open("filename", O_WRONLY); //open file in write only mode

	fcntl(fd, F_SETLKW, &flockStruct); //do the locking with fcntl
     
	//make changes to file
    int_incarNumber = getValueFromFile(filename) + 1;
    // update the incarnation number to the new value
    ofstream outFile(filename.c_str() );
    outFile << (int_incarNumber ) << endl;
    outFile.close();

	/*unlock file*/
	flockStruct.l_type = F_UNLCK; //change struct field to unlock 
	fcntl(fd, F_SETLK, &flockStruct); //unlock with fcntl()

    return(int_incarNumber);
}//end updateIncarNum

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
    int myIncNum = 0;               /* local copy of the incarnation number */
    int otherIncNum = 0;            /* copy of the incarnation number retreived from the file */
#ifdef RECEIVE_FROM_SERVER  
    int failureCount = 0;
    const int MAX_SERVER_FAILURES = 20;
    unsigned int fromSize;           /* In-out of address size for recvfrom() */
    struct sockaddr_in fromAddr;     /* Source address of echo */
	char clientString[strLen+1] = "     "; /* 5-element string belonging to the client */
    bool resendFlag = 0;
    int numChars = 0;               /* number of characters currently held in the client string */
    char echoBuffer[ECHOMAX+1];      /* Buffer for receiving echoed string */
    int respStringLen;               /* Length of received response */
#endif // RECEIVE_FROM_SERVER

    unsigned int sleepTime = 500000; // sleep time for client in microseconds (us)
    int clientNum = 0;                   /* client number, passed in as a command line argument */
	request_t clientRequest; //new reques
	//for sockopt, using sockopt to control timeout for resend of packet
	struct timeval tv;
	tv.tv_sec = 1;   // 1 Secs Timeout 
	tv.tv_usec = 0;  // 

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

    // the following will set a receive-time for the socket it is set to one second
    // from line 188; if there is not packet received after 'tv' amount of time the 
    // the control-flow moves to next line. 
    // This was chosen because it is easier to tell the socket to timeout and
    // prevent the client and server from hanging.
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	
    /* Construct the server address structure */
  	memset(&echoServAddr, 0, sizeof(echoServAddr));    /* Zero out structure */
      echoServAddr.sin_family = AF_INET;                 /* Internet addr family */
      echoServAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
      echoServAddr.sin_port   = htons(echoServPort);     /* Server port */

    // put the IP address into the structure
    strcpy(clientRequest.client_ip, getSocketIP() ); 

	//print our IP address
	printf("Client IP address = %s\n", clientRequest.client_ip);
    cout << endl;
    failurePoint = rand() % (MAX_REQUESTS) ; // the iteration at which failures will begin with a probability of 50%
    cout << "Client number " << clientNum << " will fail at iteration " << failurePoint+1 << endl;
 

	clientRequest.client = clientNum;
	for(requestNum = 0; requestNum < MAX_REQUESTS ; ++requestNum)
	{
        // reset for next request
        resendFlag = 0;

        // pack client request struct
		clientRequest.req = requestNum+1;
		// randomize the char sent to the server
		clientRequest.c   = (char) ( 97 + (rand() % 26) ); // 97 == ascii "a"

		// simulate failure modes of client
        // update the incarnation number if past the point of failure
        // get incarnation number by unlocking file, incrementing, and
        // closing file
        // client sends data even if it "fails"
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
                otherIncNum = updateIncarNum();
                myIncNum = otherIncNum;
                
                // reset the client string to empty 5 char
                strcpy(clientString, "     ");
                numChars = 0;
            }else{
                // failure did not happen, just get the current incarnation
                // number
                // no failures yet, just check that nobody else has updated it
                otherIncNum = checkIncarNum();
#ifdef DEBUG
                cout << " --> SUCCESS!! " << endl;
#endif
            }
        }else{
            // no failures yet, just check that nobody else has updated it
            otherIncNum = checkIncarNum();
        }

        if (otherIncNum != myIncNum){
            // another client has failed, this means I fail too.
            // reset the client string to empty 5 char
#ifdef DEBUG
            cout << " --> OTHER CLIENT FAILURE!! " << endl;
#endif
            strcpy(clientString, "     ");
            numChars = 0;
        }
        // update local and packet incarnation numbers
        myIncNum = otherIncNum;
        clientRequest.inc = otherIncNum;

        // update the client string with the new character, the same way the
        // server should - only once per request  
        updateClientString(clientString, clientRequest.c, strLen);
        numChars = (numChars) % (strLen) + 1; // increase numChars up to a maximum of strLen
        // purge the echoBuffer from the server for the next request
        strcpy(echoBuffer, "     ");


        do{ // send and attempt to receive from the server until the server sends a valid acknowledgement
            // client does not check for failure inside this do-while
#ifdef DEBUG
            printf("Client:: sending client data::\n");
            printRequestStructure(clientRequest);
#endif

            /* Send the string to the server */
            if (sendto(sock, &clientRequest, sizeof(clientRequest), 0, (struct sockaddr *)
                        &echoServAddr, sizeof(echoServAddr)) != sizeof(clientRequest)) {
                DieWithError("sendto() sent a different number of bytes than expected");
            }

#ifdef DEBUG  //display the string held by the client for comparison to the server string
            cout << "Client string: "  << clientString <<  endl;
#endif

#ifdef RECEIVE_FROM_SERVER
            /* immedaitely try to receive a response from the server, verifying the last packet was
             * received */
            fromSize = sizeof(fromAddr);		
            respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0,
                    (struct sockaddr *) &fromAddr, &fromSize);

#ifdef DEBUG
            cout << "Server string: " << echoBuffer << endl;
#endif


#ifdef VERBOSE
            cout << "Bytes Received = " << respStringLen << ", Bytes expected = " << sizeof(clientString) << endl;
            showSocketIP(fromAddr);
#endif
            if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr){
                fprintf(stderr,"Error: received a packet from unknown source.\n");
                showSocketIP(fromAddr);
                failureCount++;
                resendFlag=1;
            }else{

                /* null-terminate the received data */
                echoBuffer[respStringLen] = '\0';
                // if same, move on
                // else if server is older by one character, resend
                // else if server is newer by one character, move on
                if( (respStringLen == -1) ) {
                    // no packet received from the server, issue warning message
                    // and stay in while loop
                    resendFlag = 1;

                    // print error message
                    cout << "Client did not receive a response form the server, resending R=" 
                        << requestNum+1 << "<" << clientRequest.c << ">..." << endl;
                    // put the client to sleep for a bit 
#ifdef DEBUG
                    cout << "Client: sleeping for "<< sleepTime << " us...." << endl;
#endif
                    failureCount++;
                    usleep(sleepTime);
                }else{ 
#ifdef VERBOSE
                    cout << "echoBuffer=  <" << echoBuffer   << ">" << endl;
                    cout << "clientString=<" << clientString << ">" << endl;
                    cout << "numChars=" << numChars << endl;
#endif
                    for (int c = 0; c < numChars; c++) {
#ifdef VERBOSE
                        printf("client char[%d] = %c\nserver char[%d] = %c\n", c, clientString[c], c, echoBuffer[c]);
#endif
                        resendFlag = (echoBuffer[c] != clientString[c]);
                        if(resendFlag){// resendFlag=1 means a mismatch was found
#ifdef VERBOSE
                            printf("ClientString =\t<%s>\n",clientString );
                            printf("EchoBuffer =\t<%s>\n", echoBuffer);
                            printf("Client:: Mismatch found at char[%d] <%c> != <%c>\n", c, echoBuffer[c], clientString[c]);
#endif
                            failureCount++;
                            break; 
                        }
                    }            
                    // this is a valid receive - no reason to sleep or continue
                    failureCount=0;
                }
            }
         
            // if failureCount exceeds MAX_SERVER_FAILURES exit client
            if(failureCount > MAX_SERVER_FAILURES){
                printf("CLIENT: server has failed to send an ACK greater than %d times, client aborting...\n", MAX_SERVER_FAILURES);
                exit(1);   
            }
#endif // RECEIVE_FROM_SERVER
        }while ( resendFlag  ) ; 

#ifdef DEBUG
        cout << endl;
#endif
	}//end for

    close(sock);
    printf("UDPClient program successfully completed %d request iterations.\n", MAX_REQUESTS );
    exit(0);
}//end main
