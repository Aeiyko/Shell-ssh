#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct server{
    int socket;
    // int acceptedSocket;
    struct sockaddr_in servAddr;
    struct sockaddr_in clientAddr;
    socklen_t len;
} Server;

#define PORT 1344

void new_server();
int wait_client();
void *wait_request(void *arg);
void server_close();
char check_password(char *username,char *password);
