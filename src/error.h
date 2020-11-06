#pragma once
#include <stdio.h>
#include <stdlib.h>

#define ERR -1
#define NO_ERR 0
#define EXEC_CMD_ERR 1
#define FORK_ERR 2

#define syserror(n) perror(msgErr[n]),exit(n)


extern char* msgErr[];