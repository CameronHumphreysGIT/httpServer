#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h> 
#include <arpa/inet.h>
#include <errno.h>



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

    printf("%d\n", argc);
    printf("port number: %s\n", argv[1]);
    //TODO parse port number


    /* socket(), bind(), and listen() 
    have been called */
    

    //create socket using socket()
    s = socket(AF_INET, SOCK_STREAM, 0);
    printf("return value of socket(): %d\n", s);

    //bind socket using bind()

    //set hostAddress:
    /* make sure the sin_zero field is cleared */
    memset(&hostAddress, 0, sizeof(hostAddress));
    hostAddress.sin_family = AF_INET;
    hostAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // TODO see: https://stackoverflow.com/questions/19246103/socket-errorerrno-99-cannot-assign-requested-address-and-namespace-in-python
    hostAddress.sin_port = htons(5000); //TODO change this to the port parsed at the start

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
    //now use recv() or read()

    //TODO close the connection

    //send a response

    return 0;
}