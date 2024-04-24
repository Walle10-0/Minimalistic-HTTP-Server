#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <pthread.h>  // for multi-threading

#define MAXPENDING 5

void handleHTTPClient(int clntSock);

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

int setupServerSocket(unsigned short fileServPort)
{
	int servSockAddr;
	struct sockaddr_in fileServAddr; // Local address

	/* Create socket for incoming connections */
	if ((servSockAddr = socket(PF_INET, SOCK_STREAM, 0/*IPPROTO_TCP*/)) < 0)
		DieWithError("socket() failed");
  
	/* Construct local address structure */
	memset(&fileServAddr, 0, sizeof(fileServAddr));   /* Zero out structure */
	fileServAddr.sin_family = AF_INET;                /* Internet address family */
	fileServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	fileServAddr.sin_port = htons(fileServPort);      /* Local port */

	/* Bind to the local address */
	if (bind(servSockAddr, (struct sockaddr *) &fileServAddr, sizeof(fileServAddr)) < 0)
		DieWithError("bind() failed");

	/* Mark the socket so it will listen for incoming connections */
	if (listen(servSockAddr, MAXPENDING) < 0)
		DieWithError("listen() failed");
	return servSockAddr;
}

void* spawnHandlerThread(void* arg)
{
	pthread_detach(pthread_self());
	
	printf("start--------------------\n");
	
	int * clntSock = (int *)arg;
	handleHTTPClient(*clntSock);
	
	printf("end--------------------\n");
	
	pthread_exit(NULL);
}

void listenLoop(int servSock)
{
	struct sockaddr_in fileClntAddr; // Client address
	unsigned int clntLen; // Length of client address data structure
	int clntSock; // socket descriptor for client
	pthread_t ptid; // for the thread

	for (;;)
	{
		/* Set the size of the in-out parameter */
		clntLen = sizeof(fileClntAddr);

		/* Wait for a client to connect */
		if ((clntSock = accept(servSock, (struct sockaddr *) &fileClntAddr, &clntLen)) < 0)
		{
			DieWithError("accept() failed");
		}
		    
		/* clntSock is connected to a client! */
		printf("Handling client %s\n", inet_ntoa(fileClntAddr.sin_addr));

		//handleHTTPClient(clntSock);
		pthread_create(&ptid, NULL, &spawnHandlerThread, &clntSock);
	}
}

int main(int argc, char *argv[])
{
    if (argc != 2)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }

	int servSock = setupServerSocket(atoi(argv[1]));

    listenLoop(servSock);
    /* NOT REACHED */
}
