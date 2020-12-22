#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <glob.h>
#include <assert.h>
#include <sys/wait.h>
#include "commands.h"


int toArgs(char *command, char *args[BLOCK]){
  char *tmp, **current=args;
  for(tmp=strtok(command, SPACE);tmp;*current=tmp,current++,tmp=strtok(NULL, SPACE));
  return current-args;
}

int makeCd(char *cmd_tab[BLOCK], int nb_items){
    if (nb_items > 2){
      fprintf(stderr, "too much argument for cd\n");
      return CMD_ERR;
    }
    if (nb_items == 1){
      chdir(getenv("HOME"));
      return EXIT_NORMALLY;
    }
    if (nb_items == 2 && chdir(cmd_tab[1])==ERR){
      perror(msg[CMD_ERR-1]);
      return CMD_ERR;
    }
    return EXIT_NORMALLY;

}

int isWildcard(char *param){ //retourne si la chaine est contient des wildcards
  int i;
  if (IS_JOKER(*param)) return 1;
  for (i=1;i<strlen(param);i++)
    if (IS_JOKER(param[i]) && (param[i-1]!='\\')) return 1;
  return 0;
}

int wildcard(char *cmd_tab[BLOCK], int *nb_items, int tab[BLOCK]){
    int i=0, j, a=0, r; glob_t parsed;
    char *s;
    memset(tab, 0, BLOCK*sizeof(int));
    for (;i<*nb_items;){
      if (isWildcard(cmd_tab[i])){
        if (!(r=glob(cmd_tab[i], 0, NULL, &parsed))){
          memmove(cmd_tab+i+parsed.gl_pathc,cmd_tab+i+1, sizeof(char*)*(*nb_items-i));
          memcpy(cmd_tab+i, parsed.gl_pathv, sizeof(char*)*parsed.gl_pathc);
          for (j=i; j<i+parsed.gl_pathc; j++){
            tab[j] = 1;
            s = parsed.gl_pathv[a++];
            assert(s!=NULL);
            cmd_tab[j] = (char*)calloc(sizeof(char), strlen(s)+1);
            assert(cmd_tab[j] != NULL);
            assert(s!=NULL);

            memcpy(cmd_tab[j], s, sizeof(char)*strlen(s));
          }
          i+=parsed.gl_pathc;
          *nb_items += parsed.gl_pathc-1;
          globfree(&parsed);
          a=0;
        }
        else if (r==GLOB_NOMATCH){
          fprintf(stderr, "sh: no matches found or bad pattern for: %s\n", cmd_tab[i++]);
          return ERR;
        }
      }else i++;
    }
    return EXIT_NORMALLY;
}

int exec_cmd(char cmd[BLOCK]){
  int positions[BLOCK], i, nb_items, status=EXIT_NORMALLY; pid_t pid;
  char *cmd_tab[BLOCK];
  memset(cmd_tab, 0, BLOCK*(sizeof(char*)));
  nb_items = toArgs(cmd, cmd_tab);
  if (!wildcard(cmd_tab, &nb_items, positions)){
    if (!strcmp(*cmd_tab, EXIT)) return QUIT;
    if (!strcmp(*cmd_tab, "cd")){
      status=makeCd(cmd_tab, nb_items);
      return EXIT_NORMALLY;
    }
    if ((pid=fork()) == ERR){
      perror("fork error");
      return 6;
    }if (!pid){
      execvp(*cmd_tab,cmd_tab);
      perror("exec error");
      return 7;
    }
    wait(&status);
  }
  for (i=0; i<nb_items; i++) {
    if (positions[i]) free(cmd_tab[i]);
  }
  return status;
}
