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
#define PTV ";"
#define SPACE ' '
#define ET "&&"
#define OU "||"


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

int boucleCmd(char **cmd,int *sortie){
    int status,finish=1;

    pid_t pid;


    if(!strcmp(cmd[0], "exit")){ // Si exit
        *sortie = 0;
        exit(NO_ERR);
    }
    if ( (pid = fork()) == ERR ) syserror(FORK_ERR);
    else if (!pid) {
        execCmd(cmd); // Execute la commande
    }
    wait(&status);
    if (WIFEXITED(status) && WEXITSTATUS(status)!=0){
        //Changer la couleur du prompt
        finish = 0;
    }
    write(STDOUT_FILENO , "\n", sizeof(char));
    return finish;
}

int find(char **tab, char *ch){
    for (int i=0; tab[i]; i++) {
        if(!strcmp(tab[i], ch))return i;
    }
    return -1;
}

int main(void){
    
    char *ligne_cmd;
    char *tmp_cmd;
    char **cmd;
    char *separateur = PTV;
    int test_cmd,test_et = 0,test_ou = 0,test_find_et,test_find_ou;

    int sortie=1;
    while (sortie) {
        char *path = get_current_dir_name();
        write(STDOUT_FILENO, CYAND, sizeof(char)*8);
        write(STDOUT_FILENO, path, sizeof(char)*strlen(path));
        write(STDOUT_FILENO, "> ", sizeof(char)*2);
        write(STDOUT_FILENO, CYANF, sizeof(char)*4);
        free(path);

        tmp_cmd = NULL;
        ligne_cmd = NULL;
        ligne_cmd = readCmd(); // Recup la ligne
        tmp_cmd = strtok(ligne_cmd,separateur);

        cmd = NULL;
        test_et = 0;
        test_ou = 0;
        


        while (tmp_cmd) { // Tant qu'il y a des commandes dans la ligne
            if (!test_et && !test_ou) {            
                cmd = recupCmd(tmp_cmd); // Transforme la ligne en tableau
            }
            test_ou = 0;
            test_et = 0;
            if ( (test_find_et = find(cmd,ET)) >= 0) {
                test_et = 1;
                cmd[test_find_et] = NULL;
            }
            if ( (test_find_ou = find(cmd,OU)) >= 0) {
                test_ou = 1;
                cmd[test_find_ou] = NULL;

            }
            test_cmd = boucleCmd(cmd, &sortie);
            if(test_et){
                if(test_cmd){
                    cmd = &(cmd[test_find_et+1]);
                }
                else{
                    tmp_cmd = strtok(NULL, separateur);
                }
            }
            else if(test_ou){
                if (!test_cmd) {
                    cmd = &(cmd[test_find_ou+1]);     
                }
                else {
                    tmp_cmd = strtok(NULL, separateur);
                }
            }
            else {            
                tmp_cmd = strtok(NULL, separateur);
            }
        }
    }


    return 0;
    // Note todo : au lieu de faire un find ... faire que si on tombe sur une chaine && ou || regarde l etat de la derniere commande
    // et voit si on doit faire la prochaine
}