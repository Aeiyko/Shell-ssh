#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <glob.h>
#include "parser.h"
#include "commands.h"
#include "jobs.h"



int shell(){
  int end=0, i, status, size_read, positions[BLOCK];
  char*path, c='>';
  char cmd_read[BLOCK];
  last_status last;

  init_jobs();

  memset(last.cmd, 0, PART*sizeof(char));
  for(;!end;){
    memset(cmd_read, 0, BLOCK*sizeof(char));
    path = get_current_dir_name();
    if (write(STDOUT_FILENO, path, sizeof(char)*strlen(path)) == ERR) fatalerror(WRITE_ERR);
    if (write(STDOUT_FILENO, &c, sizeof(char))== ERR) fatalerror(WRITE_ERR);
    if ((size_read = read(STDIN_FILENO,&cmd_read,BLOCK*sizeof(char))) == ERR) fatalerror(READ_ERR);
    free(path);
    cmd_read[size_read-1] = '\0';
    status = exec_cmd_shell(cmd_read, &last, &end);
    if (status == QUIT) break;
  }
  return EXIT_NORMALLY;
}

int main(int argc, char* argv[], char* envp[]){
  // exit(shell());
  exec_cmd("ls");
  //init_jobs();

  /*last_status stat;
  int end=0;

  exec_cmd_2("ls *.c", &stat, &end);
  exec_cmd_2("ls|more", &stat, &end);
  exec_cmd_2("ls", &stat, &end);*/
  //exec_cmd_2("ls|cat|cat|cat|cat&&echo yes", &stat, &end);
  //exec_cmd_2("ls *.c", &stat);
  //last_status last={};
  //memset(last.cmd, 0, PART*sizeof(char));
  //char cmd[] = "ls -l | more >> f.txt | bily | ta mere | b || roger && bil | pute";
  /*struct parser p[BLOCK];
  int size=parser(cmd, p);
  int nb_pipes = count_pipes(p, 0, size);
  printf("nb pipes : %d\n", nb_pipes);*/
  //exec_cmd_2(cmd, &last);

  //struct parser pars[BLOCK];
  //parser(cmd, pars);
  //printf("%s\n", pars[0].command);
  return 0;
}
