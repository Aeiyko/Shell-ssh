#include <stdio.h>
#include <string.h>
#include "./parser.h"

#define passe_espace if(i+1 < len && cmd[i+1] == ' '){i++;for (; i < len && cmd[i] == ' '; i++);i--;}

int parser(char *cmd,struct parser parser[CH_LENGTH]){
    int ind=0;
    int len=strlen(cmd);
    int chaine;

    for (int i=0; i<len; i++) {
        switch(cmd[i]){
            case '&':
                parser[++ind].type = SEP;
                parser[ind].sep = BACK;
                if(i+1 < len && cmd[i+1] == '&'){
                    parser[ind].sep = ET;
                    i++;
                }
                break;
            case '|':
                parser[++ind].type = SEP;
                parser[ind].sep = PIPE;
                if(i+1 < len && cmd[i+1] == '|'){
                    parser[ind].sep = OU;
                    i++;
                }
                break;
            case ';':
                parser[++ind].type = SEP;
                parser[ind].sep = PTV;
                break;
            case '>':
                chaine = 0;
                parser[++ind].type = RED;
                parser[ind].red = STD;
                parser[ind].redtype = SIMPLE;
                if(i+1 < len && cmd[i+1] == '>'){
                    parser[ind].redtype = ADD;
                    i++;
                }
                if(i+1 < len && cmd[i+1] == '&'){
                    parser[ind].red = BOTH;
                    i++;
                }
                passe_espace;
                break;
            case '2':
                if( i ==0 || (cmd[i-1] == ' ' && i+1 < len && cmd[i+1] == '>') ){
                    chaine = 0;
                    parser[++ind].type = RED;
                    parser[ind].red = STDERR;
                    parser[ind].redtype = SIMPLE;
                    i++;
                    if (i+1 < len && cmd[i+1] == '>') {
                        parser[ind].redtype = ADD;
                        i++;
                    }
                    passe_espace;
                }
                break;
            case '<':
                chaine = 0;
                parser[++ind].type = RED;
                parser[ind].red = INJ;
                passe_espace;
                break;
            case '$':
                chaine = 0;
                parser[++ind].type = VAR;
                break;
            default:
                if(parser[ind].type == CMD){
                    parser[ind].command[chaine++] = cmd[i];
                }
                else if( (parser[ind].type == RED || parser[ind].type == VAR)){
                    if(cmd[i] == ' '){
                        parser[++ind].type = CMD;
                        chaine = 0;
                    }
                    parser[ind].command[chaine++] = cmd[i];
                }
                else {
                    parser[++ind].type = CMD;
                    chaine = 0;
                    parser[ind].command[chaine++] = cmd[i];
                }
                break;
        }


    }
    return ind+1;
}

void print_test(char *cmd){
    struct parser retour[CH_LENGTH];
    int taille_parse = parser(cmd, retour);
    printf("%d\n",taille_parse);
    for (int i=0; i<taille_parse; i++) {
        printf("%s ",typestring[retour[i].type]);
        if (retour[i].type == SEP) {
            printf("%s ", sepstring[retour[i].sep]);
        }
        if (retour[i].type == RED) {
            printf("%s %s ", redstring[retour[i].red], redtypestring[retour[i].redtype]);
        }

        if (retour[i].type == RED || retour[i].type == CMD || retour[i].type == VAR) {
            printf("'%s'",retour[i].command);

        }
        printf("\n");
    }
}
