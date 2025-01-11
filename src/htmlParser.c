#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/htmlParser.h"

char* read_all(int fd, int *nread){
  int bytes_read = 0, bufLength = 1024;
  int readSize = bufLength/4;
  *nread = 0;
  char * buf = malloc(bufLength);
  bytes_read = read(fd, &buf[*nread], readSize);
  while(bytes_read > 0){
    *nread += bytes_read;
    if(*nread > bufLength/2){
      bufLength *= 2;
      buf = realloc(buf, bufLength);
    }
    bytes_read = read(fd, &buf[*nread], readSize);
  }
  return buf;
}

char* createResponse(char* resource) {
    // Opening file
    FILE *file_ptr;

    // Opening file in reading mode
    file_ptr = fopen("../resources/helloWorld.html", "r");
    int fd = fileno(file_ptr);

    if (NULL == file_ptr) {
        printf("Could not find requested resource: %s\n", resource);
        return "";
    }

    // Read all
    int* nread = malloc(sizeof(int));
    char* htmlContent = read_all(fd, nread);

    // Closing the file
    fclose(file_ptr);
    char* responce = malloc(72 + *nread);
    strcpy(responce, "HTTP/1.1 200 OK\r\nContent-Length: 93\r\nContent-Type: text/html\r\n\r\n");
    strcat(responce, htmlContent);
    return responce;
}

char* parseRequest(char* request) {
    return createResponse("");
}