#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex.h>
#include "../include/htmlParser.h"
regex_t getRegex;
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
    //TODO: check for success
    char* responce = malloc(sizeof(char) * (strlen(responceHeader) + *nread + 1));
    strcpy(responce, responceHeader);
    strcat(responce, htmlContent);
    strcat(responce, "\0");
    return responce;
}

char* parseRequest(const char* request) {
  int comp = regcomp(&getRegex, "[.GET.]", 0);
  int exec = regexec(&getRegex, request, (size_t) 0, NULL, 0);
  regfree(&getRegex);
  free(request);
  if (comp == 0 && exec == 0) {
    return createResponse("helloWorld.html");
  }
  return createResponse("");
}