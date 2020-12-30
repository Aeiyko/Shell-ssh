#include <sys/types.h>
#include <sys/wait.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "jobs.h"
static list_jobs g_jobs;


void handler_childs(int code){
  job *p;
  int status, res;
  pid_t pid;
  for (p=g_jobs.first;p;){
    if ((res=waitpid(p->pid, &status, WNOHANG))){
      assert(res == p->pid);
      pid = p->pid;
      print_job_ended(*p, WIFEXITED(status)?(WEXITSTATUS(status)):(ABNORMAL));
      p=p->next;
      del_job(pid);
    }else p=p->next;
  }
}

void init_jobs(){
    g_jobs.cpt = 0;
    g_jobs.first = NULL;
    signal(SIGCHLD, handler_childs);
}

job init_job(pid_t pid, int etat, char cmd[BLOCK]){
  job j;
  memcpy(&j.cmd, cmd, sizeof(char)*BLOCK);
  j.pid = pid;
  j.etat = etat;
  j.number = -1;
  j.next = NULL;
  return j;
}

int add_job(job j){
  job *p;
  j.number = g_jobs.cpt;
  if (!g_jobs.first){
    g_jobs.first = (job*)malloc(sizeof(job));
    memcpy(g_jobs.first, &j, sizeof(job));
    g_jobs.cpt++;
    return j.number;
  }
  for (p=g_jobs.first; p->next; p=p->next);
  p->next = (job*)malloc(sizeof(job));
  memcpy(p->next, &j, sizeof(job));
  g_jobs.cpt++;
  return j.number;
}

int del_job(pid_t pid){
  job* prec = NULL, *p;
  for (p=g_jobs.first; p; prec=p, p=p->next){
    if (p->pid == pid) break;
  }
  if (!p) return 0;
  if (prec) {
    prec->next = p->next;
  }else{
    g_jobs.first = p->next;

  }
  free(p);
  if (!g_jobs.first) g_jobs.cpt = 0;
  return 1;
}

int get_job(pid_t pid, job *j){
  job *p;
  for (p=g_jobs.first; p; p=p->next){
    if (p->pid == pid){
      memcpy(j, p, sizeof(job));
      return 1;
    }
  }
  return 0;
}

int get_job_number(int number, job *j){
  job *p;
  for (p=g_jobs.first; p; p=p->next){
    if (p->number == number){
      memcpy(j, p, sizeof(job));
      return 1;
    }
  }
  return 0;
}

int get_last_job_stopped(job *j){
  job *p, *last=NULL;
  for (p=g_jobs.first; p; p=p->next){
    if (!p->etat) last = p;
  }
  if (!last) return 0;
  memcpy(j, last, sizeof(job));
  return 1;
}

int get_job_stopped_number(int number, job *j){
  job *p;
  for (p=g_jobs.first; p; p=p->next){
    if (p->number == number){
      if (!p->etat){
        memcpy(j, p, sizeof(job));
        return 1;
      }else return 0;
    }
  }
  return -1;
}

int update_job(pid_t pid, int etat){
  job *p;
  for (p=g_jobs.first; p; p=p->next){
    if (p->pid == pid){
      if (p->etat != etat){
        p->etat = etat;
        return 1;
      }
      else return 0;
    }
  }
  return 0;
}

void kill_all_jobs(){
  job *p;
  pid_t pid;
  for (p=g_jobs.first; p;){
    kill(p->pid, SIGKILL);
    pid = p->pid;
    p=p->next;
    del_job(pid);
  }
}
void print_job_ended(job j, int status){
  printf("%s (jobs=[%d], pid=%d) terminée avec status=%d\n", j.cmd, j.number, j.pid, (status==ABNORMAL)?(-1):(status));
}

void print_job(job j){
  printf("[%d] %d Etat %s\n", j.number, j.pid, (j.etat)?(EXECUTION):(STOPPED));
}

int get_last_job(job *j){
  job *p=NULL;
  for (p=g_jobs.first; p && p->next; p=p->next);
  if (!p) return 0;
  memcpy(j, p, sizeof(job));
  return 1;
}

void print_all_jobs(){
  for (job *p=g_jobs.first; p; p=p->next){
    print_job(*p);
  }
}
