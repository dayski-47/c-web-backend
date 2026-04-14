#ifndef HTTP_PARSER
#define HTTP_PARSER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
    METHOD_GET,         // 0
    METHOD_POST,        // 1   
    METHOD_PUT,         // 2
    METHOD_DELETE,      // 3
} httpMethod;

typedef struct {
    httpMethod method;
    char *path;
    char *version;
    char *host;
    char *user_agent;
    char *accept;
    char *connection;
    char *content_type;
    int   content_length;
    char *body;
} httpRequest;

int parse(char *raw, httpRequest *req); // return 0 - works, return 1 - fails

char *extract(char *s, char *delim); 
int extract_into(char **dest, char *s, char *delim);

void free_request(httpRequest *req);
#endif




