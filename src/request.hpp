///////////////////////////////////////////////////
// header file for client and server 
// 
//
//
//////////////////////////////////////////////////


#include <arpa/inet.h>
#include <ifaddrs.h>
#include <string.h>
#include <sys/types.h>

#define ACK_TO_CLIENT 1

typedef struct request
{
        char client_ip[16];  //to hold client IP address in dotted decimal
        int inc;             //incarnation number of client
        int client;          //client number
	int req;             //request number
        char c;              //random char client sends to server
}request_t;

char addressBuffer[INET_ADDRSTRLEN]; //INET_ADDRSTRLEN = 16

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

// variables common to the server and client
const short strLen = 5;

// the following are likely unused
#define ECHOMAX 255     /* Longest string to echo */
