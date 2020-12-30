#pragma once
#include <sys/types.h>
#define EXEC 1
#define STOP 2
#define BLOCK 1024

#define STOPPED "stoppé"
#define EXECUTION "en cours d'exécution"
#define ABNORMAL 127

typedef struct _job{
  char cmd[BLOCK];
  int etat;
  pid_t pid;
  int number;
  struct _job *next;
}job;

typedef struct _list_jobs{
  int cpt;
  job *first;
}list_jobs;

void init_jobs();
job init_job(pid_t pid, int etat, char cmd[BLOCK]);
void print_job_ended(job j, int status);
void print_job(job j);
void print_all_jobs();
int update_job(pid_t pid, int etat);
int add_job(job j);
int del_job(pid_t pid);
void kill_all_jobs();
int get_last_job(job *j);
int get_last_job_stopped(job *j);
int get_job_number(int number, job *j);
int get_job_stopped_number(int number, job *j);
