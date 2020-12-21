#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "./client.h"

typedef struct myssh{
    char *username;
    char password[128];
    char *host;
} Myssh;

int main(int argc,char *argv[]){
    char *completename;
    Myssh myssh;

    if(argc != 2) fprintf(stderr, "Le nombre d'arguments n'est pas valide\n"),exit(1);

    myssh.username = strtok(argv[1], "@");
    myssh.host = strtok(NULL, "@");
    printf("Password : ");
    scanf("%s",myssh.password);

    printf("%s %s %s\n",myssh.username,myssh.password,myssh.host);

    newClient(myssh.host,1344);

    send_to_server(myssh.username,myssh.password);
    wait_server_response();
    stop_client();
    return 0;
}