#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <pthread.h>
#include <signal.h>
#include <assert.h> //
#include "myssh_server.h"
#include "../commande/commands.h"

static char *signames[] = {"SIGABRT","SIGALRM","SIGFPE","SIGHUP","SIGILL","SIGINT","SIGKILL","SIGPIPE","SIGQUIT","SIGSEGV","SIGTERM","SIGUSR1","SIGUSR2"};
static int signamesvalue[] = {SIGABRT,SIGALRM,SIGFPE,SIGHUP,SIGILL,SIGINT,SIGKILL,SIGPIPE,SIGQUIT,SIGSEGV,SIGTERM,SIGUSR1,SIGUSR2};
static int g_pid;
static int g_socket;

void *wait_signal(void *arg){
    int socket = *(int *)arg;
    char signal[8] = "SIG";
    struct serversignal serversignal;
    for(;;){
        recv(socket, &serversignal, sizeof(serversignal), MSG_PEEK);
        if (serversignal.type == SSH_MSG_CHANNEL_REQUEST && !strcmp("signal", serversignal.strings)) {
            recv(socket, &serversignal, sizeof(serversignal), 0);
            strcat(signal, serversignal.strings+strlen(serversignal.strings)+1);
            for (int i=0; i < sizeof(signames)/sizeof(char *); i++) {
                if(!strncmp(signames[i],signal,8)){
                    fprintf(stderr,"Server receive a signal\n");
                    kill(g_pid, signamesvalue[i]);
                    break;
                }
            }
        }
    }
}

void pouet(int code){
    struct serversignal serversignal;
    serversignal.type = SSH_MSG_CHANNEL_SUCCESS;
    strcpy(serversignal.strings,"signalexit");
    send(g_socket, &serversignal, sizeof(serversignal), 0);
    fprintf(stderr,"Close server\n");
    close(g_socket);
    exit(0);
}

int main(int argc,char *argv[],char *envp[]){
    g_pid = getpid();
    for (int i=0; i < sizeof(signames)/sizeof(char *); i++) {
        signal(signamesvalue[i], pouet);
    }
    struct passwd *pwd = getpwuid(getuid());
    chdir(pwd->pw_dir);

    close(STDIN_FILENO);
    g_socket = atoi(argv[1]);

    pthread_t thread;
    pthread_create(&thread, NULL, wait_signal, &g_socket);

    struct serverssh serverssh;
    struct serversshresponse serversshresponse;
    struct serversignal serversignal;
    close(STDOUT_FILENO);
    if(dup(g_socket) == -1)perror("ERROR DUP"),close(g_socket),exit(1);
    // close(STDERR_FILENO);
    // if(dup(g_socket) == -1)perror("ERROR DUP"),close(g_socket),exit(1);

    for(;;){
        recv(g_socket, &serversignal, sizeof(serversignal), MSG_PEEK);
        if(serversignal.type == SSH_MSG_CHANNEL_REQUEST && !strcmp("signal", serversignal.strings))
            continue;

        recv(g_socket, &serverssh , sizeof(struct serverssh), 0);
        if(!strcmp("shell", serverssh.strings) && !strcmp("exit", serverssh.strings+strlen(serverssh.strings)+1)){
            break;
        }
        serversshresponse.retour = exec_cmd(serverssh.strings+strlen(serverssh.strings)+1);
        write(STDOUT_FILENO, "\0", 1);
        serversshresponse.type = SSH_MSG_CHANNEL_SUCCESS;

        send(g_socket, &serversshresponse, sizeof(serversshresponse), 0);
        if(!strcmp("exec", serverssh.strings))
            break;
    }

    close(g_socket);
    return 0;
}
