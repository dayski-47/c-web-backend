#include "handler.h"



static char *build_response(int status, const char *reason, const char *body){
    int body_len = body ? strlen(body) : 0;

    int len = snprintf(NULL, 0,         
        "HTTP/1.1 %d %s\r\n"
        "Content-Length: %d\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "%s",
        status, reason, body_len, body ? body : ""
    );

    char *res = malloc(len + 1);

    if (!res) return NULL;

    snprintf(res, len + 1,
        "HTTP/1.1 %d %s\r\n"
        "Content-Length: %d\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "%s",
        status, reason, body_len, body ? body : ""
    );
    return res;

}



char *handle_root(httpRequest *req){

    (void)req;
    return build_response(200, "OK", "Home");

}

char *handle_hello(httpRequest *req){

    (void)req;
    return build_response(200, "OK", "HELLO :)");
}

char *handle_not_found(httpRequest *req){

    (void)req;
    return build_response(404, "Not Found", "Not Found");
}