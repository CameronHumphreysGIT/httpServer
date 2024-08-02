#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h> 
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/time.h>

int main (int argc, char** argv) {
    //Vars:
    int serverSocket;
    struct sockaddr_in hostAddress;
    //int hostAddress_len;
    int clientSocket;
    struct sockaddr clientAddress;
    int clientAddress_len;
    long port = 5001;

    if (argc > 1) {
        printf("port number: %s\n", argv[1]);
        port = strtol(argv[1], NULL, 0);
        if (errno == ERANGE) {
            printf("Oh dear, could not convert port input to long, using port %ld", port);
        }
    }

    //create socket using socket()
    serverSocket= socket(AF_INET, SOCK_STREAM, 0);
    printf("return value of socket(): %d\n", serverSocket);

    //bind socket using bind()

    //set hostAddress:
    /* make sure the sin_zero field is cleared */
    memset(&hostAddress, 0, sizeof(hostAddress));
    hostAddress.sin_family = AF_INET;
    hostAddress.sin_addr.s_addr = inet_addr("127.10.1.6");
    hostAddress.sin_port = htons(port);

    //Setting timeout for operations
    int timeout_in_seconds = 5;//TODO set this in a config file, if set to 0 this will just block
    struct timeval tv;
    tv.tv_sec = timeout_in_seconds;
    tv.tv_usec = 0;
    setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    int bindSuccess = bind(serverSocket, (struct sockaddr *) &hostAddress, sizeof(hostAddress));
    printf("return value of bind(): %d\n", bindSuccess);
    if(bindSuccess == -1) {
        printf("Oh dear, something went wrong with bind()! %s, %d\n", strerror(errno), errno);
    }
    //listen on socket using listen()
    int listenSuccess = listen(serverSocket, 0); //expecting only 1 connection request
    printf("return value of listen(): %d\n", listenSuccess);

    //accept a connection with accept() using address to confirm that I got the connection
    clientAddress_len = sizeof(clientAddress);
    clientSocket = accept(serverSocket, &clientAddress, &clientAddress_len);

    printf("return value of accept(): %d\n", clientSocket);
    //TODO now use recv() or read()
    //TODO set things into retry loops if timeout != 0 (what about negative numbers?)

    //TODO investigate what needs to be done before recv()??? getting Transport endpoint is not connected
        //use receive once
    char messageBuffer[10] = {};//TODO change 10 to a defined global???, command line arg??
   
    int bytes = recv(clientSocket, &messageBuffer, 10, 0);
    while (bytes != 0 && bytes != -1) {
        //receive, 10 bytes at a time until all has been received
        printf("received a message, bytes: %d, content: %s\n", bytes, messageBuffer);
        //clean the buffer
        memset(messageBuffer, '\0', sizeof(messageBuffer));
        bytes = recv(clientSocket, &messageBuffer, 10, 0);
    }
    if (bytes == -1) {
        printf("Oh dear, something went wrong with recv()! %s, %d\n", strerror(errno), errno);
    }
    //put that into a loop that will wait and retry.
    


    //TODO send a response

    //first, shutdown the socket so it won't wait on data currently being received/sent
    int shutdownSuccess = shutdown(serverSocket, 2);
    printf("return value of shutdown(): %d\n", shutdownSuccess);
    if(shutdownSuccess == -1) {
        printf("Oh dear, something went wrong with shutdown()! %s, %d\n", strerror(errno), errno);
    }
    int closeSuccess = close(serverSocket);
    printf("return value of close(): %d\n", closeSuccess);
    

    return 0;
}