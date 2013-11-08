///////////////////////////////////////////////////
// header file for client and server 
// 
//
//
//////////////////////////////////////////////////

typedef struct request
{
        char client_ip[16];  //to hold client IP address in dotted decimal
        int inc;             //incarnation number of client
        int client;          //client number
	int req;             //request number
        char c;              //random char client sends to server
}request_t;
