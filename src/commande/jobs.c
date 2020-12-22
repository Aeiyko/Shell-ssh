#include <sys/types.h>
#include "jobs.h"


void print_job_ended(job j, int status){
  printf("%s (jobs=[%d], pid=%d) terminée avec status=%d\n", j.cmd, j.number, j.pid, status);
}

void print_job(job j){

}
 
