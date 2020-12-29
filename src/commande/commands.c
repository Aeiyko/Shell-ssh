#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <glob.h>
#include <assert.h>
#include <sys/wait.h>
#include "error.h"
#include "parser.h"
#include "jobs.h"
#include "commands.h"


static char* tab_builtin[] = {
  "cd",
  "status",
  "myfg",
  "mybg",
  "exit",
  "myjobs"
};

static builtin_fonc foncs[] = {
  makeCd,
  makeStatus,
  NULL,//makeMyfg,
  NULL, //makeMybg,
  makeExit,
  NULL//makeMyJobs
};

int toArgs(char *command, char *args[BLOCK]){
  char *tmp, **current=args;
  for(tmp=strtok(command, SPACE);tmp;*current=tmp,current++,tmp=strtok(NULL, SPACE));
  return current-args;
}

int makeCd(cmd_infos* infos, last_status* last, int* end){
    int nb_items = infos->nb_items;
    if (nb_items > 2){
      fprintf(stderr, "too much argument for cd\n");
      return CMD_ERR;
    }
    if (nb_items == 1){
      chdir(getenv("HOME"));
      return EXIT_NORMALLY;
    }
    if (nb_items == 2 && chdir(infos->cmd_tab[1])==ERR){
      perror(msg[CMD_ERR-1]);
      return CMD_ERR;
    }
    return EXIT_NORMALLY;
}

int makeExit(cmd_infos* infos, last_status* last, int* end){
  *end = 1;
  return EXIT_NORMALLY;
}

int makeStatus(cmd_infos* infos, last_status* last, int* end){
  if (last->status == ABNORMAL) printf("%s terminé anormalement\n", last->cmd);
  else printf("%s terminé avec comme code de retour %d\n", last->cmd, last->status);
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
            //memset(cmd_tab[j], 0, strlen(cmd_tab[j])*sizeof(char));
            cmd_tab[j] = (char*)calloc(sizeof(char), strlen(s)+1);
            memcpy(cmd_tab[j], s, sizeof(char)*(strlen(s)+1));
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

void free_cmd_tab(cmd_infos *infos){
  int i;
  for (i=0; i<infos->nb_items; i++) if (infos->positions[i]) free(infos->cmd_tab[i]);
}

int check_background(){
  return 0;
}

void copy_last_command(last_status *last, int status, char* cmd){
  last->status = status;
  memset(last->cmd, 0, sizeof(char)*PART);
  memcpy(last->cmd, cmd, (strlen(cmd)+1)*sizeof(char));
}

void prepare_infos(cmd_infos *infos){
  memset(infos->positions, 0, BLOCK*sizeof(int));
  memset(infos->cmd_tab, 0, sizeof(infos->cmd_tab));
}

int prepare_cmd(struct parser item, cmd_infos *infos){
  prepare_infos(infos);
  infos->nb_items = toArgs(item.command, infos->cmd_tab);

  if (wildcard(infos->cmd_tab, &infos->nb_items, infos->positions)){
    return PARSE_ERR;
  }


  return EXIT_NORMALLY;
}

//si ce n'est pas une commande interne -> -1
int exec_inner_cmd(cmd_infos *infos, last_status *last, int *end){
  int i, ind=-1, status;
  for (i=0; i<sizeof(tab_builtin)/sizeof(char*); i++){
    if (infos->cmd_tab[0] && !strcmp(tab_builtin[i], infos->cmd_tab[0])){
      ind=i;
      break;
    }
  }
  if (ind!=-1) return foncs[ind](infos, last, end);
  return -1;
}

int run_cmd(cmd_infos *infos, int *end){
  execvp(infos->cmd_tab[0], infos->cmd_tab);
  notiferror(EXEC_ERR, infos->cmd_tab[0]);
  return ABNORMAL;
}


int count_pipes(struct parser p[BLOCK],int start, int end){
  int i, cpt=0;
  for (i=start; i<end; i++){
    if (p[i].type == SEP && p[i].sep != PIPE) return cpt;
    else if (p[i].type == SEP) cpt++;
  }
  return cpt;
}

int exec_pipes(struct parser p[BLOCK],int start, int end, last_status *last){
  int nb_pipes = count_pipes(p, start+1, end);
  int tubes[2*nb_pipes], i, a, nb_cmd=0, status;
  int cmds[BLOCK]; cmd_infos infos;
  memset(cmds, 0, BLOCK*sizeof(int));

  pid_t pid;
  int parsed = 0;
  for (i=start; i<end; i++){
    if(p[i].type == SEP && p[i].sep != PIPE) break;
    if(p[i].type == CMD) cmds[nb_cmd++] = i;
  }
  pid_t pids[nb_cmd];
  parsed = i-start-1;
  if (!parsed) return parsed;
  for (i=0; i<nb_pipes; i++) if (pipe(tubes+2*i) == ERR) fatalerror(PIPE_ERR);
  for (i=0; i<nb_cmd; i++){
    if ((pid=fork())==ERR) fatalerror(FORK_ERR);
    if (!pid){
        if (i && dup2(tubes[(i-1)*2], STDIN_FILENO)==ERR)
          fatalerror(DUP_ERR);
        if (i<(nb_cmd-1) && dup2(tubes[(2*i)+1], STDOUT_FILENO)==ERR)
          fatalerror(DUP_ERR);
        for (a=0; a<nb_pipes*2; a++) close(tubes[a]);
        if (prepare_cmd(p[cmds[i]], &infos)){
          notiferror(PARSE_ERR, infos.cmd_tab[0]);
        }else exit(run_cmd(&infos, &end));
    }
    pids[i]=pid;
  }
  for (a=0; a<nb_pipes*2; a++){ //le père n'utilise pas ces tubes
    close(tubes[a]);
  }
  for (i=0; i<nb_cmd; i++){
    waitpid(pids[i], &status, 0);
  }
  if (WIFEXITED(status)){
    copy_last_command(last, WEXITSTATUS(status), p[cmds[nb_cmd-1]].command);
  }
  else{
    copy_last_command(last, ABNORMAL, p[cmds[nb_cmd-1]].command);
  }
  return parsed;
}

int exec_cmd(char cmd[BLOCK]){
  last_status last; int end;
  return exec_cmd_shell(cmd, &last, &end);
}

int exec_cmd_shell(char cmd[BLOCK], last_status *last, int* end){
  int i, piped, status=0, size_parse, fg=1; enum mode mod=NORMAL, nb_pipes; pid_t pid;
  int nb;
  cmd_infos infos;
  struct parser parse[BLOCK];
  struct parser current;
  memset(&current, 0, sizeof(struct parser));
  memset(parse, 0, sizeof(struct parser)*BLOCK);

  size_parse = parser(cmd, parse);

  for (i=0; i<size_parse; i++){
    memcpy(&current, parse+i, sizeof(struct parser));
    switch(current.type){
        case SEP:
          switch (current.sep){
            case ET:
              mod=ANDCOND;
              break;
            case OU:
              mod=ORCOND;
              break;
            case PTV:
              mod=NORMAL;
              break;
            case PIPE:
              break;
            case BACK:
              mod=NORMAL;
              break;
          }
        break;
        case CMD:
          if (mod==ANDCOND && status) continue;
          if (mod==ORCOND && (!status)) continue;
          piped=exec_pipes(parse, i, size_parse, last);
          i+=piped;
          if (!piped){

            if (prepare_cmd(current, &infos)){
              notiferror(PARSE_ERR, infos.cmd_tab[0]);
              break;
            }
            if ((nb=exec_inner_cmd(&infos, last, end)) != -1){

              copy_last_command(last, nb, current.command);
              break;
            }

             if ((pid=fork())==ERR) fatalerror(FORK_ERR);

             if (!pid){
               exit(run_cmd(&infos, end));
             }

            if (i+1 < size_parse && parse[i+1].type == SEP && parse[i+1].sep == BACK){

              add_job(init_job(pid, 1, current.command));
            }
            else{
            wait(&status);
             if (WIFEXITED(status)){
               copy_last_command(last, WEXITSTATUS(status), current.command);
             }
             else{
                copy_last_command(last, ABNORMAL, current.command);
             }
           }
           free_cmd_tab(&infos);
          }
          //fg = check_background(parse, size_parse, i);
          //printf("statut en %d : %d\n", i, status);
        case RED:
          break;
        case VAR:
          break;
    }
  }
  return WEXITSTATUS(status);
}
