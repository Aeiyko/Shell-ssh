#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "./client.h"

typedef struct myssh{
    char *username;
    char password[128];
    char *host;
    char *hostname;
} Myssh;

int config_file(Myssh *myssh){
    int bool=0;
    char path[1024];
    char host[1024];
    char hostname[1024];
    char user[1024];
    strcpy(path, getenv("HOME"));
    strcat(path, "/.myssh/config");
    if(!myssh->host){
        FILE *tmp_file;
        if( !(tmp_file = fopen(path, "r")) ) fprintf(stderr,"ERREUR : %s n'existe pas\n",path),exit(2);
        for(;fscanf(tmp_file, "Host %s\n\tHostname %s\n\tUser %s\n",host,hostname,user) != EOF;) {
            if(!strcmp(host, myssh->username)){
                myssh->username = malloc(sizeof(char)*1024);
                strcpy(myssh->username, user);
                myssh->host = malloc(sizeof(char)*1024);
                strcpy(myssh->host, hostname);
                myssh->hostname = malloc(sizeof(char)*1024);
                strcpy(myssh->hostname, host);
                printf("hostname : %s\n",myssh->hostname);
                bool=1;
                break;
            }
            memset(host, 0, 1024);
            memset(hostname, 0, 1024);
            memset(user, 0, 1024);
        }
        if( (fclose(tmp_file)) == EOF ) perror("Erreur de fermeture du fichier de config myssh\n"),exit(2);
    }
    return bool;
}

int main(int argc,char *argv[]){
    char *completename;
    Myssh myssh;
    char commande[1024];
    char *mode;
    int bool;

    if(argc < 2) fprintf(stderr, "Le nombre d'arguments n'est pas valide\n"),exit(1);

    myssh.username = strtok(argv[1], "@");
    myssh.host = strtok(NULL, "@");

    bool = config_file(&myssh);
    if(!myssh.host){
        printf("Connexion failed !\n");
        exit(1);
    }

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
        mode = "exec";
    }
    else if(argc == 2){
        mode = "shell";
    }
    else{
        perror("Erreur dans les arguments"),exit(1);
    }

    printf("Password : ");
    scanf("%s",myssh.password);

    printf("%s %s %s %s\n",myssh.username,myssh.password,myssh.host,myssh.hostname);

    newClient(myssh.host,1344);

    recup_signal();
    send_to_server(myssh.username,myssh.password);
    if(wait_server_response()){
        send_command_to_server(commande,mode,myssh.username,myssh.host,myssh.hostname,bool);
    }
    stop_client();
    return 0;
}