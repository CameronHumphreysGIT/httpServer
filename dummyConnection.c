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

//TODO: refactor code somehow? how to even Organize C code?
struct received{
    int bytes;
    bool received;
};

struct received receive(int clientSocket, char messageBuffer[], int bufferSize) {
    bool recvd = false;
    int bytes = recv(clientSocket, messageBuffer, bufferSize, 0);
    while (bytes != 0 && bytes != -1) {
        recvd = true;
        //receive, 10 bytes at a time until all has been received
        printf("received a message, bytes: %d, content: %s\n", bytes, messageBuffer);
        //clean the buffer
        memset(messageBuffer, '\0', (bufferSize + 1)* sizeof(char));
        bytes = recv(clientSocket, messageBuffer, bufferSize, 0);
    }
    struct received result = {bytes, recvd};
    return result;
}

int cleanup(int serverSocket) {
    //first, shutdown the socket so it won't wait on data currently being received/sent
    int shutdownSuccess = shutdown(serverSocket, 2);
    printf("return value of shutdown(): %d\n", shutdownSuccess);
    if(shutdownSuccess == -1) {
        printf("Oh dear, something went wrong with shutdown()! errno: %s, %d\n", strerror(errno), errno);
        return -1;
    }
    int closeSuccess = close(serverSocket);
    printf("return value of close(): %d\n", closeSuccess);
    if(closeSuccess == -1) {
        printf("Oh dear, something went wrong with close()! errno: %s, %d\n", strerror(errno), errno);
        return -1;
    }
    return 0;
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
    //TODO set these in a config/ or take in command line args
        const int acceptRetries = 5;
        const int recvRetries = 5;
        const int timeoutSeconds = 5;
        const int recvTries = 5;//TODO: use this somehow??
        const int bufferSize = 10;
        const char* addressString = "127.10.1.3";

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

    //bind socket using bind()10

    //set hostAddress:
    /* make sure the sin_zero field is cleared */
    memset(&hostAddress, 0, sizeof(hostAddress));
    hostAddress.sin_family = AF_INET;
    hostAddress.sin_addr.s_addr = inet_addr(addressString);
    hostAddress.sin_port = htons(port);

    struct timeval tv;
    tv.tv_sec = timeoutSeconds;
    tv.tv_usec = 0;
    setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    int bindSuccess = bind(serverSocket, (struct sockaddr *) &hostAddress, sizeof(hostAddress));
    printf("return value of bind(): %d\n", bindSuccess);
    if(bindSuccess == -1) {
        printf("Oh dear, something went wrong with bind()! errno: %s, %d\n", strerror(errno), errno);
        return cleanup(serverSocket);
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
            return cleanup(serverSocket);
        }
    }
    
    //We only want to try receiving if we successfully accepted a connection
    if (clientSocket != -1) {    
        //buffersize + 1 so if the buffer is full there can be a t
        char messageBuffer[bufferSize + 1] = {};
        //clean the message buffer
        memset(messageBuffer, '\0', (bufferSize + 1) * sizeof(char));
        struct received result = receive(clientSocket, messageBuffer, bufferSize);
        //if timeout, set into a retry loop
        if (result.bytes == -1 && timeoutSeconds > 0) {
            int retryCount = 1;
            while (errno == 11 && retryCount <= recvRetries) {
                printf("Looks like recv() timed out, retrying... errno: %s, %d\n", strerror(errno), errno);
                errno = 0;
                //try recv() again
                printf("Retry attempt %d of recv()\n", retryCount);
                struct received result = receive(clientSocket, messageBuffer, bufferSize);
                //if we received any message, rest the previous retry attempts.
                if (result.received) {
                    retryCount = 0;
                }
                retryCount++;
            }
            if (retryCount > recvRetries) {
                printf("Looks like recv() failed after %d tries. errno: %s, %d\n", recvRetries, strerror(errno), errno);
            }
        }
        if (errno != 11) {
            //TODO: process errno0 (Success) which will receive 0 bytes when client closes the connection (Ctrl+C)
            printf("Oh dear, something went wrong with recv()! errno: %s, %d\n", strerror(errno), errno);
            return cleanup(serverSocket);
        }
    }

    //TODO send a response

    //TODO: add binary to gitignore.
    return cleanup(serverSocket);
}

