#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex.h>
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
    char* resourceDir = "../resources/";
    char* file = malloc(strlen(resource) + strlen(resourceDir));
    strcpy(file, resourceDir);
    strcat(file, resource);
    file_ptr = fopen(file, "r");
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
    char* responceHeader = "HTTP/1.1 200 OK\r\nContent-Length: 93\r\nContent-Type: text/html\r\n\r\n";
    char* responce = malloc(strlen(responceHeader) + *nread);
    strcpy(responce, responceHeader);
    strcat(responce, htmlContent);
    return responce;
}

char* parseRequest(char* request) {
  regex_t getRegex;
  int value;
 
  // Function call to create regex
  value = regcomp( &getRegex, "GET", 0);
  if (value == 0) {
    value = regexec( &getRegex, request, 0, NULL, 0);
    if (value == 0) {
      return createResponse("helloWorld.html");
    }
  }
  return createResponse("");
}