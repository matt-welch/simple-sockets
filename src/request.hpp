///////////////////////////////////////////////////
// header file for client and server 
// 
//
//
//////////////////////////////////////////////////

/* common libraries to server and client */
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <string>
using std::string;

/* libraries needed only for functions defined in request.hpp */
#include <ifaddrs.h>
#include <sys/types.h>

#define ACK_TO_CLIENT 1
// variables common to the server and client
const short strLen = 5;

typedef struct request
{
        char client_ip[16];  //to hold client IP address in dotted decimal
        int inc;             //incarnation number of client
        int client;          //client number
	int req;             //request number
        char c;              //random char client sends to server
}request_t;

typedef struct compact_request_data{
    int req;    /* client request number */
    char clientString[strLen+1]; /* current client string held by the server */
}request_data_t;

char addressBuffer[INET_ADDRSTRLEN]; //INET_ADDRSTRLEN = 16
char addressStr[INET_ADDRSTRLEN]; //INET_ADDRSTRLEN = 16

void DieWithError(const char *errorMessage) /* External error handling function */
{
    perror(errorMessage);
    exit(1);
}

// function to get the IP address out of a sockaddr_in structure
char* showSocketIP(struct sockaddr_in& host_addr){
    strcpy(addressStr, inet_ntoa(host_addr.sin_addr));
#ifdef DEBUG
    printf("showSocketIP: IP addr: %s\n", addressStr);
    printf("showSocketIP: port is: %d\n", (int) ntohs(host_addr.sin_port));
#endif
    return addressStr;
}



// function to get the IP address in use by a socket
char* getSocketIP()
{
    struct ifaddrs* ifAddress = NULL; //struct used to store network info from getifaddrs
    struct ifaddrs* ifa = NULL;	//used to traverse linked-list
    void* tmpAddr = NULL;

    getifaddrs(&ifAddress); //creates a linked-list of structs describing network interfaces
    //addresses are stored in ifaddrs->ifa_addr, names in ifaddrs->ifa_name

    //traverse through linked-list
    for (ifa = ifAddress; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr->sa_family == AF_INET)//AF_INET is address family used for sockets
        {
            tmpAddr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;//cast to get ip of socket
            //inet_ntop converts binary addresses to text
            inet_ntop(AF_INET, tmpAddr, addressBuffer, INET_ADDRSTRLEN);
            if(strcmp(ifa->ifa_name, "lo"))
            {
                return addressBuffer;
            }
        }
    }
    return NULL;
}

void updateClientString(char* myString, char c, int len){
    // function copies characters from left to right (0= left) so that the
    // newer character may be placed in myString[0]
	for (int i = len-1; i > 0; i--) {// terminates when i=1, copying myString[0] to myString[1]
		myString[i] = myString[i-1];
    }
	myString[0] = c;
	return;
}

void printRequestStructure(request_t clientRequest){
    printf("\tclientRequest.client_ip(char*)\t= %s\n", clientRequest.client_ip);
    printf("\tclientRequest.inc (int)\t\t= %d\n", clientRequest.inc);
    printf("\tclientRequest.client (int)\t= %d\n", clientRequest.client);
    printf("\tclientRequest.req (int)\t\t= %d\n", clientRequest.req);
    printf("\tclientRequest.c (char)\t\t= %c\n", clientRequest.c);
}

// the following are likely unused
#define ECHOMAX 6     /* Longest string to echo */
