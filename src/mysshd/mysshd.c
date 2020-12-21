#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/socket.h>
#include "server.h"
#define MAX 500

int main(int argc,char *argv[]){
    char buf[MAX];
    char *msg=NULL;
    memset(buf,0,sizeof(char)*MAX);
    
    new_server();
    

    printf("Le serveur écoute\n"); 
    wait_client();
    printf("billy accepté\n");        

    wait_request();

    server_close();
 
    return 0;
}