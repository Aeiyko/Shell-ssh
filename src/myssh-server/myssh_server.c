#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc,char *argv[]){
    int socket;
    socket = atoi(argv[1]);
    printf("ALED\n");
    char pp[4]="yop";
    if (send(socket, pp, 4, 0) == -1)perror("Error send");
    close(socket);
    return 0;
}