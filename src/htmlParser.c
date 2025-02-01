#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex.h>
#include "../include/htmlParser.h"
#define RESPONSEHEADERLEN 100

regex_t getRegex;


char* readAll(int fd, int *nread){
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

char* getResponseHeader(int responseType, int contentLength) {
  char* responceHeader = malloc(sizeof(char) * RESPONSEHEADERLEN);
  if (responceHeader == NULL) {
    return "";
  }
  switch (responseType) {
    case 200:
      sprintf(responceHeader, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: text/html\r\n\r\n", contentLength);
      return responceHeader;
    case 404:
    //TODO: fix this ContentLength not matching problem
      sprintf(responceHeader, "HTTP/1.1 404 Not Found\r\nContent-Length: %d\r\nContent-Type: text/html\r\n\r\n", contentLength + 1);
      return responceHeader;
    default:
      return "";
  }
}

char* createResponse(char* resource, int responceType) {
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
    char* htmlContent = readAll(fd, nread);
    // Closing the file
    fclose(file_ptr);
    char* responceHeader = getResponseHeader(responceType, *nread);
    //TODO: check for success
    char* responce = malloc(sizeof(char) * (strlen(responceHeader) + *nread + 1));
    strcpy(responce, responceHeader);
    strcat(responce, htmlContent);
    strcat(responce, "\0");
    return responce;
}

char* parseRequest(const char* request) {
  const int nMatches = 1;
  regmatch_t regMatch[nMatches];
  //TODO: fix this regex
  const int comp = regcomp(&getRegex, "(GET).*", 0);
  const int exec = regexec(&getRegex, request, nMatches, regMatch, 0);
  regfree(&getRegex);
  free(request);
  if (comp == 0 && exec == 0) {
    return createResponse("helloWorld.html", 200);
  }
  return createResponse("resourceNotFound.html", 404);
}