#include <htmlParser.h>

char* parseRequest(char* request) {
    return "HTTP/1.1 200 OK\r\nContent-Length: 93\r\nContent-Type: text/html\r\n\r\n<html><body>Hi</body></html>";
}