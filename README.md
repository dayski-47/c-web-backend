📺 c_web_framework

Lightweight HTTP server written in C from scratch using Winsock. 
Supports HTTP/1.1 request parsing, routing and dynamic reponse generation.

Features
+ Raw TCP socket server with Winsock2
+ HTTP/1.1 request parser (method, path, version, headers, body)
+ Simple router with handler functions
+ Supports GET method
+ Multiple clients via select()

Requirements 
+ Windows
+ MinGW or MSVC compiler

Build

With MinGW:

```MinGW
gcc main.c server.c http_parser.c handler.c router.c -o server.exe -lws2_32
```
With MSVC:
```MSVC
cl main.c server.c http_parser.c handler.c router.c /link ws2_32.lib
```

Run

```cmd
server.exe
```
Server will start accepting connections on 127.0.0.1:36172

USAGE 
```cmd
curl http://127.0.0.1:36172/
curl http://127.0.0.1:36172/hello
```

Routes
| Method | Path     | Response      |
|--------|----------|---------------|
| GET    | `/`      | 200 Home      |
| GET    | `/hello` | 200 HELLO :)  |
| *      | *        | 404 Not Found |