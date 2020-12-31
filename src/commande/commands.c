#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <glob.h>
#include <assert.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "error.h"
#include "parser.h"
#include "jobs.h"
#include "commands.h"

static pid_t g_pid_foreground = NONE;
static char* g_command = NULL;
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
  makeMyfg,
  makeMybg,
  makeExit,
  makeMyJobs
};

void handler_ctrl_c(int code){
    if (g_pid_foreground!=NONE){
      kill(g_pid_foreground, SIGINT);
    }
    else{
      kill_all_jobs();
      ask_for_quit();
    }
}

void handler_ctrl_z(int code){
  int nb;
  if (g_pid_foreground!=NONE){
    printf("\n"); //pour le buffer

    nb = add_job(init_job(g_pid_foreground, 0, g_command));
    printf("La commande %s devient le job %d et il est stoppé\n", g_command, nb);
    kill(g_pid_foreground, SIGTSTP);
    g_pid_foreground = NONE;
    g_command = NULL;
  }
}

void redirect_signals(){
  signal(SIGINT, handler_ctrl_c);
  signal(SIGTSTP, handler_ctrl_z);
}

void ask_for_quit(){
  char c;
  char ask[] = ASK_QUIT;
  do{
    printf("\n");
    if (write(STDOUT_FILENO, ask, sizeof(ask))==ERR) fatalerror(WRITE_ERR);
    if (read(STDIN_FILENO, &c, sizeof(char))==ERR) fatalerror(READ_ERR);
    if (c=='y') exit(0);
  }while (c!='y' && c!='n');
}

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

int makeMyJobs(cmd_infos* infos, last_status* last, int* end){
  print_all_jobs();
  return EXIT_NORMALLY;
}

int makeMyfg(cmd_infos* infos, last_status* last, int* end){
  job j;
  if (infos->nb_items == 2){
    int number = (int)strtol(infos->cmd_tab[1], NULL,10);
    if (!get_job_number(number, &j)){
      notiferror(JOB_ERR, ": ce job n'existe pas !");
      return JOB_ERR;
    }
    kill(j.pid, SIGCONT);
    g_pid_foreground = j.pid;
    g_command = j.cmd;
    del_job(j.pid);
    return EXIT_NORMALLY;
  }else if (infos->nb_items == 1){
    if (!get_last_job(&j)){
      notiferror(JOB_ERR, ": ce job n'existe pas !");
      return JOB_ERR;
    }
    kill(j.pid, SIGCONT);
    g_pid_foreground = j.pid;
    g_command = j.cmd;
    del_job(j.pid);

    return EXIT_NORMALLY;
  }

  else return PARSE_ERR;
}

int makeMybg(cmd_infos* infos, last_status* last, int* end){
  job j;
  int nb;
  if (infos->nb_items == 2){
    int number = (int)strtol(infos->cmd_tab[1], NULL,10);
    nb=get_job_stopped_number(number, &j);
    if (nb==-1){
      notiferror(JOB_ERR, ": ce job n'existe pas ou n'est pas stoppé !");
      return JOB_ERR;
    }else if (!nb){
      notiferror(JOB_ERR, ": ce job est déjà en cours d'exécution !");
      return JOB_ERR;
    }
    kill(j.pid, SIGCONT);
    return EXIT_NORMALLY;
  }else if (infos->nb_items == 1){
    if (!get_last_job_stopped(&j)){
      notiferror(JOB_ERR, ": pas de job stoppé en arrière-plan !");
      return JOB_ERR;
    }
    kill(j.pid, SIGCONT);
    return EXIT_NORMALLY;
  }

  else return PARSE_ERR;
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
          return GLOB_NOMATCH;
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
  int res;
  prepare_infos(infos);
  infos->nb_items = toArgs(item.command, infos->cmd_tab);

  res = wildcard(infos->cmd_tab, &infos->nb_items, infos->positions);
  if (res==GLOB_NOMATCH) return GLOB_NOMATCH;
  if (res) return PARSE_ERR;
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
  return EXEC_ERR;
}


int count_pipes(struct parser p[BLOCK], int start, int end){
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
  int parsed = 0, result;
  for (i=start; i<end; i++){
    if (p[i].type != SEP && p[i].type != CMD) break;
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
        if ((result = prepare_cmd(p[cmds[i]], &infos))){
          if (result != GLOB_NOMATCH)
            notiferror(PARSE_ERR, infos.cmd_tab[0]);
        }else exit(run_cmd(&infos, &end));
    }
    pids[i]=pid;
  }
  for (a=0; a<nb_pipes*2; a++){ //le père n'utilise pas ces tubes
    close(tubes[a]);
  }
  for (i=0; i<nb_cmd; i++){
    g_pid_foreground = pids[i];
    g_command = p[cmds[i]].command;
    waitpid(pids[i], &status, (i==nb_cmd-1)?(0):(WUNTRACED));
  }
  g_pid_foreground = NONE;
  g_command = NULL;
  if (WIFEXITED(status)){
    copy_last_command(last, WEXITSTATUS(status), p[cmds[nb_cmd-1]].command);
  }
  else if (!WIFSTOPPED(status)){
    copy_last_command(last, ABNORMAL, p[cmds[nb_cmd-1]].command);
  }
  return parsed;
}

int exec_cmd(char *cmd){
  char buff[BLOCK];
  memset(buff,0,BLOCK);
  strcpy(buff,cmd);
  last_status last; int end = 0;
  memset(last.cmd,0,PART*sizeof(char));
  return exec_cmd_shell(buff, &last, &end);
}

void make_redirections(struct parser *item, int size_parse){
  int fd;
  //command contient le nom de fichier dans le cas d'une redirection
   if (item->red != INJ){
    if (item->redtype == ADD){
      if((fd=open(item->command, 0655 | O_APPEND | O_WRONLY))==ERR){
        if((fd=open(item->command,0655 | O_TRUNC | O_CREAT | O_WRONLY))==ERR) fatalerror(OPEN_ERR);
      }
    }
    else{
      if((fd=open(item->command,0655 | O_TRUNC | O_CREAT | O_WRONLY))==ERR){
        if ((fd=open(item->command,0655 | O_TRUNC | O_WRONLY))==ERR) fatalerror(OPEN_ERR);
      }
    }
    switch(item->red){
      case STD:
        if (dup2(fd, STDOUT_FILENO) == ERR) fatalerror(DUP_ERR);
        break;
      case STDERR:
        if (dup2(fd, STDERR_FILENO) == ERR) fatalerror(DUP_ERR);
        break;
      case BOTH:
        if (dup2(fd, STDOUT_FILENO) == ERR) fatalerror(DUP_ERR);
        if (dup2(fd, STDERR_FILENO) == ERR) fatalerror(DUP_ERR);
        break;
      default:
        fatalerror(PARSE_ERR);
        break;
     }
   }
   else{
     if ((fd=open(item->command,O_RDONLY))==ERR) fatalerror(OPEN_ERR);
     if (dup2(fd, STDIN_FILENO)==ERR) fatalerror(DUP_ERR);
   }

}

int exec_cmd_shell(char *cmd, last_status *last, int* end){
  int i, piped, status=0, size_parse, fg=1; enum mode mod=NORMAL, nb_pipes; pid_t pid;
  int nb, result;
  char buff[BLOCK];
  memset(buff,0,BLOCK);
  strcpy(buff,cmd);

  cmd_infos infos;
  struct parser parse[BLOCK];
  struct parser current;
  memset(&current, 0, sizeof(struct parser));
  memset(parse, 0, sizeof(struct parser)*BLOCK);

  size_parse = parser(buff, parse);

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
          if (mod==ANDCOND && last->status) continue;
          if (mod==ORCOND && (!last->status)) continue;

          piped=exec_pipes(parse, i, size_parse, last);
          i+=piped;

          if (!piped){
            if ((result = prepare_cmd(current, &infos))){
              if (result != GLOB_NOMATCH) notiferror(PARSE_ERR, infos.cmd_tab[0]);
              break;
            }
            if ((nb=exec_inner_cmd(&infos, last, end)) != -1){
              copy_last_command(last, nb, current.command);
              if (g_pid_foreground != NONE){
                waitpid(g_pid_foreground, &status, WUNTRACED);
              }
              break;
            }
            if ((pid=fork())==ERR) fatalerror(FORK_ERR);
            if (!pid) {

              if (i+1 < size_parse && parse[i+1].type == RED){
                make_redirections(parse+i+1, size_parse);
              }

              exit(run_cmd(&infos, end));
            }
            g_pid_foreground = pid;
            g_command = current.command;
            if (i+1 < size_parse && parse[i+1].type == SEP && parse[i+1].sep == BACK){
              add_job(init_job(pid, 1, current.command));
            }
            else{

            waitpid(pid, &status, WUNTRACED);
            g_pid_foreground = NONE;
            g_command = NULL;
             if (WIFEXITED(status)){
               copy_last_command(last, WEXITSTATUS(status), current.command);
             }
             else if (!WIFSTOPPED(status)){
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
