#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h> 
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>


int main (int argc, char** argv) {
    //Vars:
    int s;
    struct sockaddr_in hostAddress;
    int bindSuccess;
    //int hostAddress_len;
    int listenSuccess;
    int clientSocket;
    struct sockaddr clientAddress;
    int clientAddress_len;
    long port = 5001;

    if (argc > 1) {
        printf("port number: %s\n", argv[1]);
        port = strtol(argv[1], NULL, 0);
        if (errno == ERANGE) {
            printf("Oh dear, could not convert port input to long, using port 5000");
        }
    }

    //create socket using socket()
    s = socket(AF_INET, SOCK_STREAM, 0);
    printf("return value of socket(): %d\n", s);

    //bind socket using bind()

    //set hostAddress:
    /* make sure the sin_zero field is cleared */
    memset(&hostAddress, 0, sizeof(hostAddress));
    hostAddress.sin_family = AF_INET;
    hostAddress.sin_addr.s_addr = inet_addr("127.10.1.5");
    hostAddress.sin_port = htons(port);

    bindSuccess = bind(s, (struct sockaddr *) &hostAddress, sizeof(hostAddress));
    printf("return value of bind(): %d\n", bindSuccess);
    if(bindSuccess == -1) {
        printf("Oh dear, something went wrong with bind()! %s, %d\n", strerror(errno), errno);
    }
    //listen on socket using listen()
    listenSuccess = listen(s, 0); //expecting only 1 connection request
    printf("return value of listen(): %d\n", listenSuccess);

    //accept a connection with accept() using address to confirm that I got the connection
    clientAddress_len = sizeof(clientAddress);
    clientSocket = accept(s, &clientAddress, &clientAddress_len);

    printf("return value of accept(): %d\n", clientSocket);
    //TODO now use recv() or read()

    //TODO send a response

    //first, shutdown the socket so it won't wait on data currently being received/sent
    int shutdownSuccess = shutdown(s, 2);
    printf("return value of shutdown(): %d\n", shutdownSuccess);
    if(shutdownSuccess == -1) {
        printf("Oh dear, something went wrong with shutdown()! %s, %d\n", strerror(errno), errno);
    }
    int closeSuccess = close(s);
    printf("return value of close(): %d\n", closeSuccess);
    

    return 0;
}