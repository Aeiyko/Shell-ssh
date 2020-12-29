#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <shadow.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include "server.h"
#include "./error.h"
#include "./sshstruct.h"

static Server srv;

void new_server(){
    int sfd;
    if((sfd=socket(AF_INET,SOCK_STREAM,0))==ERR){
        syserror(SOCKET_ERR);
    }

    srv.socket=sfd;
    memset(&srv.servAddr,0,sizeof(struct sockaddr_in));
    memset(&srv.clientAddr,0,sizeof(struct sockaddr_in));
    srv.len=sizeof(struct sockaddr_in);

    srv.servAddr.sin_family = AF_INET;
    srv.servAddr.sin_addr.s_addr = INADDR_ANY;
    srv.servAddr.sin_port = htons((uint16_t) PORT);
    if (bind(srv.socket, (struct sockaddr *) &srv.servAddr, sizeof(srv.servAddr)) < 0) {
        syserror(BINDING_ERR);
        server_close();
    }

    if(listen(srv.socket,SOMAXCONN)==-1){
                exit(1);
        }
}

int wait_client(){
    return accept(srv.socket,(struct sockaddr*)&srv.clientAddr,&srv.len);
}

void *wait_request(void *arg){
    struct passwd *password;
    int acceptedSocket = *(int *)arg;
    struct ssh ssh;
    int induser = 0,indpwd;
    char response;
    if(recv(acceptedSocket, &ssh, sizeof(struct ssh), 0) == ERR){
        syserror(READ_ERR);
        server_close();
    }
    printf("----------\n");
    printf("type de requete : %d\n",ssh.user_request);
    int ind=0;
    printf("Utilisateur : %s\n",ssh.strings+ind);
    ind += strlen(ssh.strings)+1;
    printf("mode : %s\n",ssh.strings+ind);
    ind += strlen(ssh.strings+ind)+1;
    printf("securité utilisé : %s\n",ssh.strings+ind);
    ind += strlen(ssh.strings+ind)+1;
    printf("Mot de passe : %s\n",ssh.strings+ind);
    printf("----------\n");

    indpwd=ind;
    response = check_password(&ssh.strings[induser], &ssh.strings[indpwd])?SSH_MSG_USERAUTH_SUCCESS:SSH_MSG_USERAUTH_FAILURE;

    send(acceptedSocket, &response, 1, 0);

    if(response == SSH_MSG_USERAUTH_SUCCESS){
        pid_t pid;
        password = getpwnam(&ssh.strings[induser]);
        if (!(pid=fork())) {
            setuid(password->pw_uid);
            char sock[64];
            sprintf(sock,"%d",acceptedSocket);
            execl("bin/myssh_server", "bin/myssh_server",sock,NULL);
            fprintf(stderr, "ERROR FORK\n");
        }
    }


    return NULL;
}

char check_password(char *username,char *password){
    struct spwd *myuser = getspnam(username);
    if(myuser){
        return !strcmp(myuser->sp_pwdp,crypt(password, myuser->sp_pwdp));

    }
    else
        fprintf(stderr,"Check password : user not existing\n");
    return 0;
}

void server_close(){
    close(srv.socket);
}
