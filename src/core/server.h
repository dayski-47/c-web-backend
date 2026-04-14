#ifndef SERVER_H
#define SERVER_H

// windows lean and mean strips out rarely used stuff from windows.h
// must be defined BEFORE including windows headers
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>   // core winsock — sockets, select, send, recv
#include <windows.h>    // general windows API
#include <ws2tcpip.h>   // getpeername, inet_ntoa, address structs
#include <iphlpapi.h>   // IP helper functions
#include <stdio.h>
#include <string.h>     // memset, memcmp, strlen

#include "http_parser.h"
#include "router.h"
#include "handler.h"
// ------------------------------------------------
// config
// ------------------------------------------------
#define BUFLEN      512
#define PORT        36172
#define ADDRESS     "127.0.0.1"
#define MAX_CLIENTS 30

// ------------------------------------------------
// server state
// ------------------------------------------------
typedef struct {
    SOCKET   listener;
    SOCKET   clients[MAX_CLIENTS];
    int      curNoClients;
    char     on;
} ServerState;

// ------------------------------------------------
// functions 
// ------------------------------------------------
int  server_init(ServerState *s);   // WSAStartup, socket, bind, listen
void server_run(ServerState *s);    // the main loop
void server_shutdown(ServerState *s); // cleanup, goodbye messages, WSACleanup

#endif // SERVER_H