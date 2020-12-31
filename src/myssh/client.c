#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include "client.h"
#include "./error.h"
#include "../mysshd/sshstruct.h"
#include "../myssh-server/myssh_server.h"

#define neterr_client(clt, n) client_destroy(clt),syserror(n);

static Client clt;

static char *signames[] = {"ABRT","ALRM","FPE","HUP","ILL","INT","KILL","PIPE","QUIT","SEGV","TERM","USR1","USR2"};
static int signamesvalue[] = {SIGABRT,SIGALRM,SIGFPE,SIGHUP,SIGILL,SIGINT,SIGKILL,SIGPIPE,SIGQUIT,SIGSEGV,SIGTERM,SIGUSR1,SIGUSR2};
static int g_envoye = 0;

// void _exit(int status){
//     fprintf(stderr,"stop client\n");
//     stop_client();
// }

void traite_signal(int code){
    struct serversignal serversignal;
    struct serversignal serversignalreceiv;
    serversignal.type = SSH_MSG_CHANNEL_REQUEST;
    strcpy(serversignal.strings, "signal");
    for (int i=0; i<sizeof(signames)/sizeof(char *); i++) {
        if(code == signamesvalue[i])
            strcpy(serversignal.strings+strlen(serversignal.strings)+1, signames[i]);
    }
    send(clt.socket, &serversignal, sizeof(serversignal), 0);
    printf("\nJ AI ENVOYE AU SERVEUR %d %s %s\n",serversignal.type,serversignal.strings,serversignal.strings+strlen(serversignal.strings)+1);
    recv(clt.socket, &serversignalreceiv, sizeof(serversignalreceiv), MSG_PEEK);
    if(serversignalreceiv.type == SSH_MSG_CHANNEL_SUCCESS && !strcmp(serversignalreceiv.strings, "signalexit")){
        // _exit(0);
        fprintf(stderr,"stop client\n");
        stop_client();
        exit(0);
    }
    else {
        if (!g_envoye && (code == SIGINT || code == SIGKILL) ) {
            // _exit(0);
            // exit(0);
            fprintf(stderr,"stop client\n");
            stop_client();
            exit(0);
        }
    }
}

void recup_signal(){
    for (int i=0; i<sizeof(signames)/sizeof(char*); i++) {
        signal(signamesvalue[i],traite_signal);
    }
}

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

int wait_server_response(){
    struct ssh ssh;
    recv(clt.socket, &ssh, sizeof(struct ssh), 0);
    if(ssh.user_request == SSH_MSG_USERAUTH_SUCCESS){
        printf("Connection success\n");
        return 1;
    }
    else if (ssh.user_request == SSH_MSG_USERAUTH_FAILURE) {
        printf("Connection failed\n");
    }
    else {
        printf("Error unknown response : %d\n",ssh.user_request);
    }
    return 0;

}

void stop_client(){
    close(clt.socket);

}

void recup_cmd(char *commande){
    char c;
    int i=0;
    while ((c=getchar()) != '\n') {
        commande[i++] = c;
    }
}

void vider_buffer(){
    char c;
    do {
        c = getchar();
    }while (c != '\n' && c!='\0');
}

void affiche_prompt(char *username,char *host,char *hostname,int boolean){
    // char *path = get_current_dir_name();
    // if (write(STDOUT_FILENO, path, sizeof(char)*strlen(path)) == ERR) perror("ERROR PATH"),exit(1);
    if (write(STDOUT_FILENO, username, strlen(username))== ERR) perror("ERROR PATH"),exit(1);
    if (write(STDOUT_FILENO, "@", sizeof(char))== ERR) perror("ERROR PATH"),exit(1);
    if(boolean){
        if (write(STDOUT_FILENO, hostname, strlen(host))== ERR) perror("ERROR PATH"),exit(1);
    }
    else {
        if (write(STDOUT_FILENO, host, strlen(host))== ERR) perror("ERROR PATH"),exit(1);
    }
    if (write(STDOUT_FILENO, " > ", sizeof(char)*3)== ERR) perror("ERROR PATH"),exit(1);
    // free(path);
}

void send_command_to_server(char *cmd,char *mode,char *username,char *host,char *hostname,int boolean){
    struct serverssh serverssh;
    struct serversshresponse serversshresponse;
    struct serversignal serversignalreceiv;
    char response[1024];
    char commande[1024];
    ssize_t readed;
    char c;

    strcpy(commande, cmd);
    memset(response, 0, sizeof(response));
    serverssh.type = SSH_MSG_CHANNEL_REQUEST;
    strcpy(serverssh.strings, mode);
    strcpy(&serverssh.strings[strlen(mode)+1], commande);


    vider_buffer();

    for(;;){
        g_envoye=0;
        if(!strcmp(mode,"shell")){
            affiche_prompt(username,host,hostname,boolean);
            memset(commande, 0, sizeof(commande));
            recup_cmd(commande);
            strcpy(&serverssh.strings[strlen(mode)+1], commande);
        }

        // recv(clt.socket, &serversignalreceiv, sizeof(serversignalreceiv), MSG_PEEK);
        // if(serversignalreceiv.type == SSH_MSG_CHANNEL_SUCCESS && !strcmp(serversignalreceiv.strings, "signalexit")){
        //     // _exit(0);
        //     fprintf(stderr,"stop client\n");
        //     stop_client();
        //     exit(0);
        // }
        send(clt.socket, &serverssh, sizeof(struct serverssh), 0);
        g_envoye = 1;
        if(!strcmp("shell", mode) && !strcmp(commande, "exit"))
            break;

        do {
            memset(response, 0, 1024);
            readed = recv(clt.socket,response,1024,MSG_PEEK);
            printf("%s",response);
        }while (readed == 1024 && recv(clt.socket,response,1024,0));

        recv(clt.socket, response, strlen(response)+1, 0);
        do {
            recv(clt.socket, &c, 1, MSG_PEEK);
        }while (!c && recv(clt.socket, &c, 1, 0));

        memset(&serversshresponse, 0, sizeof(serversshresponse));
        recv(clt.socket, &serversshresponse, sizeof(struct serversshresponse ), 0);
        printf("\nProcessus distant terminé avec le code [%d]\n",serversshresponse.retour);
        if(!strcmp(mode,"exec"))
            break;
    }
}
