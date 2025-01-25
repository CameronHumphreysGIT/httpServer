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
#include <getopt.h>
#include "../include/htmlParser.h"
//TODO: set this in a config, or figure out how to let requestString be flexible somehow?
#define REQUESTBUFFERSIZE 1000


//TODO: refactor code somehow? how to even Organize C code?
struct received{
    int bytes;
    bool received;
    char* requestString;
};

struct received receiveUntilTimeout(int clientSocket, char messageBuffer[], int bufferSize) {
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
    //TODO: consider killing this method? also set MessageBuffer somehow
    struct received result = {bytes, recvd};
    return result;
}

struct received receiveOneRequest(int clientSocket, char messageBuffer[], int bufferSize) {
    bool recvd = false;
    int totalBytes = 0;
    char* request = malloc(sizeof(char) * REQUESTBUFFERSIZE);
    if (request == NULL) {
        struct received result = {0, false, NULL};
        return result;
    }
    char* test = request;
    int bytes = recv(clientSocket, messageBuffer, bufferSize, 0);
    while (bytes != 0 && bytes != -1 && totalBytes < REQUESTBUFFERSIZE) {
        recvd = true;
        //receive, 10 bytes at a time until all has been received
        if (totalBytes + bytes >= REQUESTBUFFERSIZE) {
            int writeBytes = REQUESTBUFFERSIZE - totalBytes - 1;
            memset(&messageBuffer[writeBytes], '\n', sizeof(char));
        }
        printf("received a message, bytes: %d, content: %s\n", bytes, messageBuffer);
        //copy over to the request buffer
        totalBytes += bytes;
        strcat(request, messageBuffer);
        if (messageBuffer[bytes - 1] == '\n') {
            break;
        }
        //clean the buffer
        memset(messageBuffer, '\0', (bufferSize + 1)* sizeof(char));
        bytes = recv(clientSocket, messageBuffer, bufferSize, 0);
    }
    struct received result = {bytes, recvd, request};
    return result;
}

int cleanup(int serverSocket) {
    //first, shutdown the socket so it won't wait on data currently being received/sent
    int shutdownSuccess = shutdown(serverSocket, SHUT_RDWR);
    printf("return value of shutdown(): %d\n", shutdownSuccess);
    if(shutdownSuccess == -1) {
        printf("Oh dear, something went wrong with server shutdown()! errno: %s, %d\n", strerror(errno), errno);
        return -1;
    }
    int closeSuccess = close(serverSocket);
    printf("return value of close(): %d\n", closeSuccess);
    if(closeSuccess == -1) {
        printf("Oh dear, something went wrong with server close()! errno: %s, %d\n", strerror(errno), errno);
        return -1;
    }
    return 0;
}

int cleanupClientAndServer(int serverSocket, int clientSocket) {
    int closeSuccess = close(clientSocket);
    printf("return value of close(): %d\n", closeSuccess);
    if(closeSuccess == -1) {
        printf("Oh dear, something went wrong with client close()! errno: %s, %d\n", strerror(errno), errno);
        return -1;
    }
    //then, shutdown the server socket so it won't wait on data currently being received/sent
    return cleanup(serverSocket);
}

void printErrors(int bytes, int recvRetries) {
    switch(errno) {
        case 0:
            printf("The client has closed the connection bytes: %d, errno: %s, %d\n", bytes, strerror(errno), errno);
            break;
        case 11:
            //when the connection is established, but closed serverside, 
            printf("Looks like recv() failed after %d tries. errno: %s, %d\n", recvRetries, strerror(errno), errno);
            break;
        default:
            printf("Oh dear, something went wrong with recv()! errno: %s, %d\n", strerror(errno), errno);
            break;
    }
}

void printHelp() {
    // Opening file
    FILE *file_ptr;

    // String buffer that stores the read character
    char str[50];

    // Opening file in reading mode
    file_ptr = fopen("../resources/help.txt", "r");

    if (NULL == file_ptr) {
        printf("help.txt file can't be opened \n");
        return;
    }

    // Reading string using fgets
    while (fgets(str, 50, file_ptr) != NULL) {
        printf("%s", str);
    }

    // Closing the file
    fclose(file_ptr);
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
    //TODO set these in a config and take in command line args, also how to make the program silent?
        const int acceptRetries = 5;
        const int recvRetries = 5;
        const int timeoutSeconds = 5;
        const int messageBufferSize = 10;
        const char* addressString = "127.10.1.3";
        const int requestBufferSize = 100;

    if (argc > 1) {
        int opt = getopt(argc, argv, "p:h");
        switch (opt) {
        case 'p':
            port = strtol(optarg, NULL, 0);
            if (errno == ERANGE) {
                printf("Oh dear, could not convert port input to  a long value, using port %ld", port);
            }
            break;
        case 'h':
            printHelp();
            break;
        default:
            fprintf(stderr, "Usage: %s [-p port] [-h]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    //create socket using socket()
    serverSocket= socket(AF_INET, SOCK_STREAM, 0);
    printf("return value of socket(): %d\n", serverSocket);

    //set hostAddress:
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
    //TODO revise the structure of this code.
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
            return cleanup(serverSocket);
        }
        if (errno != 11 && errno != 0) {
            printf("Oh dear, something went wrong with accept()! errno: %s, %d\n", strerror(errno), errno);
            return cleanup(serverSocket);
        }
    }
    struct received result;
    //We only want to try receiving if we successfully accepted a connection
    if (clientSocket != -1) {    
        //buffersize + 1 so if the buffer is full there is still an end character so printf doesn't print forever.
        char messageBuffer[messageBufferSize + 1] = {};
        //clean the message buffer
        memset(messageBuffer, '\0', (messageBufferSize + 1) * sizeof(char));
        result = receiveOneRequest(clientSocket, messageBuffer, messageBufferSize);
        //we will start a retry loop if timeout
        int retryCount = 1;
        if (timeoutSeconds > 0) {
            while (errno == 11 && retryCount <= recvRetries) {
                printf("Looks like recv() timed out, retrying... errno: %s, %d\n", strerror(errno), errno);
                errno = 0;
                //try recv() again
                printf("Retry attempt %d of recv()\n", retryCount);
                result = receiveOneRequest(clientSocket, messageBuffer, messageBufferSize);
                //if we received any message, rest the previous retry attempts.
                if (result.received) {
                    retryCount = 0;
                }
                retryCount++;
            }
        }
        //This case means that there has been some kind of issue
        if (result.bytes == 0 || result.bytes == -1) {
            printErrors(result.bytes, recvRetries);
            return cleanupClientAndServer(serverSocket, clientSocket);
        }
    }
    //check if received has been initialized

    printf("Received Request: %s\n", result.requestString);
    char* message = parseRequest(result.requestString);
    printf("Sending Message: %s\n", message);
    int sendSuccess = send(clientSocket, message, 142 * sizeof(char), 0);
    free(message);
    printf("return value of send(): %d\n", sendSuccess);
    if(sendSuccess == -1) {
        printf("Oh dear, something went wrong with send()! errno: %s, %d\n", strerror(errno), errno);
        return cleanupClientAndServer(serverSocket, clientSocket);
    }
    sleep(5);
    return cleanupClientAndServer(serverSocket, clientSocket);
}

