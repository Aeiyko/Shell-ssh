#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include "myssh_server.h"
#include "../commande/commands.h"

int main(int argc,char *argv[],char *envp[]){
    struct passwd *pwd = getpwuid(getuid());
    chdir(pwd->pw_dir);

    close(STDIN_FILENO);
    int socket;
    socket = atoi(argv[1]);
    struct serverssh serverssh;
    struct serversshresponse serversshresponse;
    close(STDOUT_FILENO);
    if(dup(socket) == -1)perror("ERROR DUP"),close(socket),exit(1);
    close(STDERR_FILENO);
    if(dup(socket) == -1)perror("ERROR DUP"),close(socket),exit(1);

    for(;;){
        recv(socket, &serverssh , sizeof(struct serverssh), 0);
        if(!strcmp("shell", serverssh.strings) && !strcmp("exit", serverssh.strings+strlen(serverssh.strings)+1)){
            break;
        }
        serversshresponse.retour = exec_cmd(serverssh.strings+strlen(serverssh.strings)+1);
        write(STDOUT_FILENO, "\0", 1);
        serversshresponse.type = SSH_MSG_CHANNEL_SUCCESS;

        send(socket, &serversshresponse, sizeof(serversshresponse), 0);
        if(!strcmp("exec", serverssh.strings))
            break;
    }

    close(socket);
    return 0;
}
