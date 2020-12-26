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
    char commande[1024];

    if(argc < 2) fprintf(stderr, "Le nombre d'arguments n'est pas valide\n"),exit(1);

    myssh.username = strtok(argv[1], "@");
    myssh.host = strtok(NULL, "@");

    if(argc > 2 && !strcmp(argv[2],"-c")){
        int indice = 0; 
        // 
        int i;
        for (i=3; i<argc-1; i++) {
            strcpy(commande+indice, argv[i]);
            indice += strlen(argv[i]);
            commande[indice++] = ' ';  
        }
        strcpy(commande+indice, argv[i]);
        
    }
    else{
        perror("Erreur dans les arguments"),exit(1);
    }

    printf("Password : ");
    scanf("%s",myssh.password);

    printf("%s %s %s\n",myssh.username,myssh.password,myssh.host);

    newClient(myssh.host,1344);

    send_to_server(myssh.username,myssh.password);
    if(wait_server_response()){
        send_command_to_server(commande,"exec");
    }
    stop_client();
    return 0;
}