#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "error.h"

#define CYAND "\033[01;36m"
#define CYANF "\033[0m"
#define CH_LENGTH 1024
#define OPT_LENGTH 42
#define SEP ";"
#define SPACE ' '


char * readCmd(){ // Lis une ligne entière
    int i = 0,limit = CH_LENGTH;
    char c;
    
    char *cmd = malloc(CH_LENGTH*sizeof(char)+1);
    cmd[CH_LENGTH] = '\0';


    read(STDIN_FILENO, &c, sizeof(char));
    while (c != '\n') {
        cmd[i++] = c;
        if(strlen(cmd) == limit){
            limit*=2;
            cmd = realloc(cmd, sizeof(char)*limit+1);
            cmd[limit] = '\0';
        }
        read(STDIN_FILENO, &c, sizeof(char));
    }
    cmd = realloc(cmd, sizeof(char)*i+1);
    cmd[i] = '\0';

    return cmd;
}

void execCmd(char **command){
    execvp(command[0], command); // Execute la commande
    syserror(EXEC_CMD_ERR);
}

char **recupCmd(char *command){
    int nbspace=0,cpt=0;
    for (int i=0; command[i] != '\0'; i++) { // Compte le nombre d'espace
        if (command[i] == SPACE){
            nbspace++;
        }
    }
    char ** cmd = malloc(sizeof(char *)*(nbspace+2));
    char *last = command;
    int j;
    for (j=0; command[j] != '\0'; j++) { // Transforme la chaine en un tableau de chaine séparer par des espaces
        if(command[j] == SPACE){ 
            command[j] = '\0';
            cmd[cpt++] = last;
            last = &(command[j+1]);
        }
    }
    command[j] = '\0';
    cmd[cpt++] = last;
    cmd[cpt] = NULL;
    return cmd;
}


int main(void){
    
    pid_t pid;
    int status;
    char *ligne_cmd;
    char *tmp_cmd;
    char **cmd;

    int sortie=1;
    while (sortie) {
        char *path = get_current_dir_name();
        write(STDOUT_FILENO, CYAND, sizeof(char)*8);
        write(STDOUT_FILENO, path, sizeof(char)*strlen(path));
        write(STDOUT_FILENO, "> ", sizeof(char)*2);
        write(STDOUT_FILENO, CYANF, sizeof(char)*4);
        free(path);

        ligne_cmd = readCmd(); // Recup la ligne
        tmp_cmd = strtok(ligne_cmd,SEP);

        while (tmp_cmd) { // Tant qu'il y a des commandes dans la ligne
            cmd = recupCmd(tmp_cmd); // Transforme la ligne en tableau
            if(!strcmp(cmd[0], "exit")){ // Si exit
                sortie = 0;
                exit(NO_ERR);
            }
            if ( (pid = fork()) == ERR ) syserror(FORK_ERR);
            else if (!pid) {
                execCmd(cmd); // Execute la commande
            }
            wait(&status);
            if (WIFEXITED(status) && WEXITSTATUS(status)!=0){
                //Changer la couleur du prompt
            }
            write(STDOUT_FILENO , "\n", sizeof(char));
            tmp_cmd = strtok(NULL, SEP);    
        }
    }


    return 0;
}