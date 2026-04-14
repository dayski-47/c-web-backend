#ifndef HANDLER
#define HANDLER


#include "router.h"
#include <stdio.h>

char* handle_root(httpRequest *req);
char* handle_hello(httpRequest *req);
char* handle_not_found(httpRequest *req);

static char *build_response(int status, const char *reason, const char *body);


#endif