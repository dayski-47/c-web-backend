#ifndef ROUTER_H
#define ROUTER_H

#include <stdio.h>
#include <string.h>

#include "http_parser.h"

typedef char* (*handler_func)(httpRequest *req);

handler_func route(httpRequest *req);

char* handle_root(httpRequest *req);
char* handle_hello(httpRequest *req);
char* handle_not_found(httpRequest *req);





#endif

