#include "http_parser.h"


char *extract(char *s, char *delim){
    char *res = strstr(s, delim);
    if (res == NULL) { return NULL; }
    
    int idx = res - s;
    
    char *out = malloc(idx + 1);
    if (out == NULL) { return NULL; }
    for (int i = 0; i < idx; i++){
        out[i] = s[i];
    }
    out[idx] = '\0';
    return out;
}

int extract_into(char **dest, char *s, char *delim){
    *dest = extract(s, delim);
    if (*dest == NULL) {
        return 1;
    } 
    return 0;
}



int parse(char *raw, httpRequest *req){
    
    if (raw == NULL || *raw == '\0') return 1;

    // Detect method
    if (!strncmp(raw, "GET ", 4)) {
        req->method = METHOD_GET;
    } else if (!strncmp(raw, "POST ", 5)){
        req->method = METHOD_POST;
    } else if (!strncmp(raw, "PUT ", 4)) {
        req->method = METHOD_PUT;
    } else if (!strncmp(raw, "DELETE ", 7)){
        req->method = METHOD_DELETE;
    } else {
        return 1;
    }

    switch (req->method) {

    case METHOD_GET:
        raw += 4;
        break;

    case METHOD_POST:
        raw += 5;
        break;

    case METHOD_PUT:
        raw += 4;
        break;

    case METHOD_DELETE:
        raw += 7;
        break;

    default:
        return 1;
    }

    req->content_length = -1; 

    // PATH
    if (extract_into(&req->path, raw, " ")) return 1;
    raw += strlen(req->path) + 1;

    // VERSION
    if (extract_into(&req->version, raw, "\r\n")) return 1;
    raw += strlen(req->version) + 2;

    // HEADERS
    while (strncmp(raw, "\r\n", 2) != 0) {

        if (!strncmp(raw, "Host: ", 6)) {
            raw += 6;
            if (extract_into(&req->host, raw, "\r\n")) return 1;

        } else if (!strncmp(raw, "Accept: ", 8)){
            raw += 8;
            if (extract_into(&req->accept, raw, "\r\n")) return 1;

        } else if (!strncmp(raw, "User-Agent: ", 12)){
            raw += 12;
            if (extract_into(&req->user_agent, raw, "\r\n")) return 1;
        } else if (!strncmp(raw, "Content-Type: ", 14)){
            raw += 14;
            if (extract_into(&req->content_type, raw, "\r\n")) return 1;
        } else if (!strncmp(raw, "Content-Length: ", 16)){
            raw += 16;
            char *temp = extract(raw, "\r\n");
            
            sscanf(temp, "%d", &req->content_length);
            free(temp);
        }

        char *next = strstr(raw, "\r\n");
        if (!next) return 1;
        raw = next + 2;
    }
    
    // BODY 
    if (req->method == METHOD_POST || req->method == METHOD_PUT){ 
        raw += 2;
        if (req->content_length == -1){
            return 1;
        }
        if (req->content_length > 0) {
            req->body = malloc(req->content_length + 1);

            for (int i = 0; i < req->content_length; i++){
                req->body[i] = raw[i];
            }
            req->body[req->content_length] = '\0'; 
        } else {
            req->body = NULL;
        }
    }


    return 0;
}

void free_request(httpRequest *req){
        if (req->path)          free(req->path);
        if (req->version)       free(req->version);
        if (req->host)          free(req->host);
        if (req->user_agent)    free(req->user_agent);
        if (req->accept)        free(req->accept);
        if (req->connection)    free(req->connection);
        if (req->content_type)  free(req->content_type);
        if (req->body)          free(req->body);    
}