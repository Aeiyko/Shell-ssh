#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <sys/socket.h>
#include "client.h"
#include "./error.h"
#include "../mysshd/sshstruct.h"

#define neterr_client(clt, n) client_destroy(clt),syserror(n);

static Client clt;

void newClient(char* addr,int port){
    int sfd;
    if((sfd=socket(AF_INET,SOCK_STREAM,0))==ERR){
        stop_client(clt.socket);
        syserror(SOCKET_ERR);
    }

    memset(&clt.clientAddr,0,sizeof(struct sockaddr_in));

    clt.socket=sfd;
    clt.clientAddr.sin_family=AF_INET;
    clt.clientAddr.sin_port=htons((uint16_t) port);
    clt.len = sizeof(struct sockaddr_in);
    if(!inet_aton(addr,&clt.clientAddr.sin_addr)){
        stop_client(clt.socket);
        syserror(SOCKET_ERR);
    }

    if(connect(clt.socket,(struct sockaddr *)&clt.clientAddr,clt.len)== ERR){
        stop_client(clt.socket);
        syserror(CONNECT_ERR);
    }
    printf("Client running(into the 90's)\n");
}

void send_to_server(char *username,char *password){
    struct ssh ssh;
    int ind=0;
    ssh.user_request = SSH_MSG_USERAUTH_REQUEST;
    strcpy(ssh.strings+ind, username);
    ind += strlen(username)+1;
    strcpy(ssh.strings+ind, "ssh");
    ind += 4;
    strcpy(ssh.strings+ind, "password");
    ind += 9;
    strcpy(ssh.strings+ind, password);
    ind += strlen(password)+1;
    send(clt.socket, &ssh, ind+1, 0);
}

void wait_server_response(){
    struct ssh ssh;
    recv(clt.socket, &ssh, sizeof(struct ssh), 0);
    if(ssh.user_request == SSH_MSG_USERAUTH_SUCCESS){
        printf("Connection success\n");
    }
    else if (ssh.user_request == SSH_MSG_USERAUTH_FAILURE) {
        printf("Connection failed\n");
    }
    else {
        printf("Error unknown response : %d\n",ssh.user_request);
    }
}

void stop_client(){
    close(clt.socket);

}