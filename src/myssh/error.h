#pragma once

#define ERR -1
#define NO_ERR 0
#define SIGEMPTYSET_ERR 1
#define SIGACTION_ERR 2
#define SOCKET_ERR 3
#define BINDING_ERR 4
#define SEND_ERR 5
#define ACCEPT_ERR 6
#define LISTEN_ERR 7
#define READ_ERR 8
#define THREAD_CREATE_ERR 9
#define OPEN_FILE_ERR 10
#define CONNECT_ERR 11

#define syserror(n) perror(msgErr[n]),exit(n)

extern char* msgErr[];
