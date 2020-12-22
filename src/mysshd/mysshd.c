#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <sys/socket.h>
#include "server.h"
#define MAX 500

int main(int argc,char *argv[]){
    pthread_t thread;
    int acceptedSocket;
    char buf[MAX];
    char *msg=NULL;
    memset(buf,0,sizeof(char)*MAX);
    
    new_server();
    

    printf("Le serveur écoute\n"); 
    
    for(;;){
        acceptedSocket = wait_client();
        printf("connexion accepté\n");        
        pthread_create(&thread, NULL, wait_request, &acceptedSocket);
        pthread_detach(thread);
        // wait_request();
    }
    server_close();
 
    return 0;
}