///////////////////////////////////////////////////
// header file for client and server 
// 
//
//
//////////////////////////////////////////////////

#define ACK_TO_CLIENT 1

typedef struct request
{
        char client_ip[16];  //to hold client IP address in dotted decimal
        int inc;             //incarnation number of client
        int client;          //client number
	int req;             //request number
        char c;              //random char client sends to server
}request_t;


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
