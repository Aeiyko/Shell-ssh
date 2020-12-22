#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <glob.h>
#include "commands.h"

int shell(){
  int end=0, i, status, size_read, positions[BLOCK];
  char*path, c='>';

  for(;!end;){
    char cmd_read[BLOCK];
    path = get_current_dir_name();
    if (write(STDOUT_FILENO, path, sizeof(char)*strlen(path)) == ERR) fatalerror(WRITE_ERR);
    if (write(STDOUT_FILENO, &c, sizeof(char))== ERR) fatalerror(WRITE_ERR);
    if ((size_read = read(STDIN_FILENO,&cmd_read,BLOCK*sizeof(char))) == ERR) fatalerror(READ_ERR);
    free(path);
    cmd_read[size_read-1] = '\0';
    status = exec_cmd(cmd_read);
    if (status == QUIT) break;
  }
  return EXIT_NORMALLY;
}

int main(int argc, char* argv[], char* envp[]){
  exit(shell());
}
