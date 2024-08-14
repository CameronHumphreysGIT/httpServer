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

//TODO: test accept and recv timouts, address warnings.
int receive(int clientSocket, char messageBuffer[], int buffersize) {
    int bytes = recv(clientSocket, messageBuffer, buffersize, 0);
    while (bytes != 0 && bytes != -1) {
        //receive, 10 bytes at a time until all has been received
        printf("received a message, bytes: %d, content: %s\n", bytes, messageBuffer);
        //clean the buffer
        memset(messageBuffer, '\0', sizeof(messageBuffer));
        bytes = recv(clientSocket, messageBuffer, buffersize, 0);
    }
    return bytes;
}


int main (int argc, char** argv) {
    //Vars:
    int serverSocket;
    struct sockaddr_in hostAddress;
    //int hostAddress_len;
    int clientSocket;
    struct sockaddr clientAddress;
    int clientAddressLength;
    long port = 5001;
    int acceptRetries = 5;//TODO set this in a config
    int recvRetires = 5;//TODO set this in a config/command line args


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
    hostAddress.sin_addr.s_addr = inet_addr("127.10.1.6");//TODO set this in config/commandline args
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
        printf("Oh dear, something went wrong with bind()! errno: %s, %d\n", strerror(errno), errno);
        return -1;
    }
    //listen on socket using listen()
    int listenSuccess = listen(serverSocket, 0); //expecting only 1 connection request
    printf("return value of listen(): %d\n", listenSuccess);

    //accept a connection with accept() using address to confirm that I got the connection
    clientAddressLength = sizeof(clientAddress);
    clientSocket = accept(serverSocket, &clientAddress, &clientAddressLength);
    printf("return value of accept(): %d\n", clientSocket);
    //check if error with accept
    if(clientSocket == -1) {
        int retryCount = 1;
        while (errno == 11 && retryCount <= acceptRetries) {
            printf("Looks like accept() timed out, retrying... errno: %s, %d\n", strerror(errno), errno);
            errno = 0;
            //try accept() again
            clientSocket = accept(serverSocket, &clientAddress, &clientAddressLength);
            printf("Retry attempt %d return value of accept(): %d\n", retryCount, clientSocket);
            retryCount++;
        }
        if (retryCount > acceptRetries) {
            printf("Looks like accept() failed after %d tries. errno: %s, %d\n", acceptRetries, strerror(errno), errno);
        }
        if (errno != 11 && errno != 0) {
            printf("Oh dear, something went wrong with accept()! errno: %s, %d\n", strerror(errno), errno);
        }
    }
    
    if (clientSocket != -1){
        //We only want to try receiving if we successfully accepted a connection
        //TODO set things into retry loops if timeout != 0 (what about negative numbers?)
        //use receive once
        char messageBuffer[10] = {};//TODO change 10 to a defined global???, command line arg??

        //TODO make a recv() function
        int bytes = receive(clientSocket, messageBuffer, 10);
        if (bytes == -1) {
            int retryCount = 1;
            while (errno == 11 && retryCount <= recvRetires) {
                printf("Looks like recv() timed out, retrying... errno: %s, %d\n", strerror(errno), errno);
                errno = 0;
                //try accept() again
                printf("Retry attempt %d of recv(): %d\n", retryCount);
                //TODO call recv() function
                bytes = receive(clientSocket, messageBuffer, 10);
                retryCount++;
            }
            if (retryCount > recvRetires) {
                printf("Looks like recv() failed after %d tries. errno: %s, %d\n", recvRetires, strerror(errno), errno);
            }
            if (errno != 11) {
                printf("Oh dear, something went wrong with recv()! errno: %s, %d\n", strerror(errno), errno);
            }
        }
        //put that into a loop that will wait and retry.
    }

    //TODO send a response


    //TODO put shutdown into a function and call it before returning -1 when errors are encountered.
    //first, shutdown the socket so it won't wait on data currently being received/sent
    int shutdownSuccess = shutdown(serverSocket, 2);
    printf("return value of shutdown(): %d\n", shutdownSuccess);
    if(shutdownSuccess == -1) {
        printf("Oh dear, something went wrong with shutdown()! errno: %s, %d\n", strerror(errno), errno);
        return -1;
    }
    int closeSuccess = close(serverSocket);
    printf("return value of close(): %d\n", closeSuccess);
    

    return 0;
}

