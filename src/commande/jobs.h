#pragma once
#include <sys/types.h>
#define EXEC 1
#define STOP 2

typedef struct _job{
  char* cmd;
  int etat;
  pid_t pid;
  int number;
  struct _job *next;
}job;

typedef struct _list_jobs{
  int cpt;
  job *first;
}list_jobs;

void print_job_ended(job j, int status);
void print_job(job j);
void print_all_jobs(list_jobs* list);
void add_job(list_jobs* list, job j);
void del_job(list_jobs* list, pid_t pid);
