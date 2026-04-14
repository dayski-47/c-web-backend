#include "router.h"


handler_func route(httpRequest *req){

    if (req->method == METHOD_GET && (!strcmp(req->path, "/"))){
        return handle_root;
    } else if (req->method == METHOD_GET && (!strcmp(req->path, "/hello"))){
        return handle_hello;
    } else { return handle_not_found;
    }
    
    

}

    


    