#pragma once
#include <stdio.h>
#include <stdlib.h>
//A alex...Falque
#define ERR -1

//A moi
#define ABNORMAL 127
#define READ_ERR 1
#define WRITE_ERR 2
#define CMD_ERR 3
#define FORK_ERR 4
#define PIPE_ERR 5
#define EXEC_ERR 6
#define PARSE_ERR 7
#define DUP_ERR 8
#define JOB_ERR 9
#define OPEN_ERR 10


#define fatalerror(n) perror(msg[n-1]), exit(n);
#define notiferror(n, s) fprintf(stderr, msg[n-1], s)
extern char* msg[];
