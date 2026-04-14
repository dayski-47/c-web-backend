#include "server.h"
// ------------------------------------------------
// INITIALIZATION
// WSAStartup, socket, bind, listen
// returns 0 on success, 1 on failure
// ------------------------------------------------
int server_init(ServerState *s) {
 
    int result;
 
    // start up winsock
    WSADATA wsaData; // configuration data
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result) {
        fprintf(stderr, "Startup Failed: %d\n", result);
        return 1;
    }
 
    // version check — make sure we got winsock 2.2
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        fprintf(stderr, "Version 2.2 of Winsock not available.\n");
        WSACleanup();
        return 1;
    }
 
    // construct socket
    s->listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // ipv4
    if (s->listener == INVALID_SOCKET) {
        printf("Error with construction: %d\n", WSAGetLastError());
        closesocket(s->listener);
        WSACleanup();
        return 1;
    }
 
    // setup for multiple connections
    char multiple = !0;
    result = setsockopt(s->listener, SOL_SOCKET, SO_REUSEADDR, &multiple, sizeof(multiple));
    if (result < 0) {
        printf("Multiple client setup failed: %d\n", WSAGetLastError());
        closesocket(s->listener);
        WSACleanup();
        return 1;
    }
 
    // bind to address
    struct sockaddr_in address;
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = inet_addr(ADDRESS);
    address.sin_port        = htons(PORT); // convert port number to byte order
    result = bind(s->listener, (struct sockaddr*)&address, sizeof(address)); // reads address as a whole sockaddr struct
    if (result == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(s->listener);
        WSACleanup();
        return 1;
    }
 
    // set as a listener
    result = listen(s->listener, SOMAXCONN); // listens to binded address, maximum connections
    if (result == SOCKET_ERROR) {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(s->listener);
        WSACleanup();
        return 1;
    }
 
    // clear client array
    memset(s->clients, 0, MAX_CLIENTS * sizeof(SOCKET));
    s->curNoClients = 0;
    s->on           = !0;
 
    printf("Accepting on %s:%d\n", ADDRESS, PORT);
    return 0;
}
 
// ------------------------------------------------
// MAIN LOOP
// handles new connections and client activity
// ------------------------------------------------
void server_run(ServerState *s) {
 
    int result, sendResult;
 
    fd_set socketSet;           // set of clients
    SOCKET sd, max_sd;
    struct sockaddr_in clientAddr;
    int clientAddrlen;
 
    char recvbuf[BUFLEN];
 
    while (s->on) {
 
        /*---------------
        BUILDING THE LIST
        -----------------*/
        FD_ZERO(&socketSet); // clear set
 
        FD_SET(s->listener, &socketSet); // adding the listening socket
 
        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = s->clients[i]; // socket
 
            if (sd > 0) {                    // check for active clients
                FD_SET(sd, &socketSet);      // add an active client to the set
            }
 
            if (sd > max_sd) {
                max_sd = sd; // select() needs the biggest socket
            }
        }
 
        /*---------------
          FREEZE THREAD
        -----------------*/
        int activity = select(max_sd + 1, &socketSet, NULL, NULL, NULL);
        if (activity < 0) {
            continue;
        }
 
        /*---------------
           NEW CLIENTS
        -----------------*/
        if (FD_ISSET(s->listener, &socketSet)) {
 
            // accept connection
            sd = accept(s->listener, NULL, NULL);
            if (sd == INVALID_SOCKET) {
                printf("Error accepting: %d\n", WSAGetLastError());
                continue;
            }
 
            // get client info
            getpeername(sd, (struct sockaddr*)&clientAddr, &clientAddrlen);
            printf("Client connected at %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
 
            if (s->curNoClients >= MAX_CLIENTS) {
                printf("Full\n");
                shutdown(sd, SD_BOTH);
                closesocket(sd);
            } else {
                // scan through list
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (!s->clients[i]) {   // adding client to open slot
                        s->clients[i] = sd;
                        printf("Added to the list at index %d\n", i);
                        s->curNoClients++;
                        break;
                    }
                }
            }
        }
 
        /*---------------
          CLIENT ACTIVITY
        -----------------*/
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!s->clients[i]) {
                continue;
            }
 
            sd = s->clients[i];
 
            // check client activity
            if (FD_ISSET(sd, &socketSet)) {
                
                result = recv(sd, recvbuf, BUFLEN, 0); // recieve request
 
                if (result > 0) {
                    httpRequest req = {0};
                    int fail = parse(recvbuf, &req);
                    
                    if (fail) {
                        // HTTP 400 Bad Request
                        shutdown(sd, SD_BOTH);
                        closesocket(sd);
                        s->clients[i] = 0;
                        s->curNoClients--;
                    } else {
                        
                        handler_func h = route(&req); 
                        char *response = h(&req); // generate response
                        if (response == NULL){
                            shutdown(sd, SD_BOTH);
                            closesocket(sd);
                            s->clients[i] = 0;
                            s->curNoClients--;
                        } else {
    
                            sendResult = send(sd, response, strlen(response), 0);
                            
                            if (sendResult == SOCKET_ERROR){
                                printf("Failed to Send: %d", WSAGetLastError());
                                shutdown(sd, SD_BOTH);
                                closesocket(sd);
                                s->clients[i] = 0;
                                s->curNoClients--;
                            }

                            free(response);
                        }
                    }

                    free_request(&req);
                
                } 

                // client closed successfully
                if (result == 0){
                    getpeername(sd, (struct sockaddr*)&clientAddr, &clientAddrlen);
                    printf("Client disconnected at %s:%d\n", inet_ntoa(clientAddr.sin_addr), 
                            ntohs(clientAddr.sin_port));                    
                    shutdown(sd, SD_BOTH);
                    closesocket(sd);
                    s->clients[i] = 0;
                    s->curNoClients--;
                    continue;
                }
                
                // error
                if (result < 0) {
                    shutdown(sd, SD_BOTH);
                    closesocket(sd);
                    s->clients[i] = 0;
                    s->curNoClients--;
                    continue;
                }

            }
        }


    }
}
 
// ------------------------------------------------
// CLEANUP
// sends goodbye to all clients, closes everything
// ------------------------------------------------
void server_shutdown(ServerState *s) {
 
    int sendResult;
 
    char *goodbye      = "Goodbye";
    int   goodbyeLength = strlen(goodbye);
 
    // close listener first so no new connections come in
    closesocket(s->listener);
 
    // sign off all connected clients
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (s->clients[i] > 0) {
            sendResult = send(s->clients[i], goodbye, goodbyeLength, 0);
            shutdown(s->clients[i], SD_BOTH);
            closesocket(s->clients[i]);
            s->clients[i] = 0;
        }
    }
 
    WSACleanup();
    printf("Shutting Down\n");
}
 
// ------------------------------------------------
// ENTRY POINT
// ------------------------------------------------
int main() {
    ServerState s;
 
    if (server_init(&s) != 0) {
        return 1;
    }
 
    server_run(&s);
    server_shutdown(&s);
 
    return 0;
}
 