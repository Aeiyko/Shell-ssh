#pragma once
#include "parser.h"
#include "error.h"

#define EXIT_NORMALLY 0
#define QUIT -2


enum mode{NORMAL, ORCOND, ANDCOND};

#define BLOCK 1024
#define PART 256

#define EXIT "exit"
#define SPACE " "

#define IS_JOKER(c) (c=='?' || c=='*' || c=='[')

typedef struct{
  char cmd[PART];
  int status;
}last_status;

typedef struct{
  char *cmd_tab[BLOCK];
  int positions[BLOCK];
  int nb_items;
}cmd_infos;

typedef int (*builtin_fonc)(cmd_infos*, last_status*, int*);

int toArgs(char *command, char *args[BLOCK]);
int makeCd(cmd_infos* infos, last_status* last, int* end);
int makeStatus(cmd_infos* infos, last_status* last, int* end);
int makeMyfg(cmd_infos* infos, last_status* last, int* end);
int makeMybg(cmd_infos* infos, last_status* last, int* end);
int makeExit(cmd_infos* infos, last_status* last, int* end);
int makeMyJobs(cmd_infos* infos, last_status* last, int* end);
int wildcard(char *cmd_tab[BLOCK], int *nb_items, int tab[BLOCK]);
int exec_cmd(char cmd[BLOCK]);
int exec_cmd_shell(char cmd[BLOCK], last_status *last, int *end); 
int count_pipes(struct parser p[BLOCK],int start, int end);
