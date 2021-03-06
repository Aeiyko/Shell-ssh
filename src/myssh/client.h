#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct client{
    int socket;
    struct sockaddr_in clientAddr;
    socklen_t len;
} Client;

void newClient(char* addr,int port);
void send_to_server(char *username,char *password);
void stop_client();
int wait_server_response();
void send_command_to_server(char *cmd, char *mode,char *username,char *host,char *hostname,int boolean);
void recup_cmd(char *commande);
void vider_buffer();
void affiche_prompt(char *username,char *host,char *hostname,int boolean);
void recup_signal();
