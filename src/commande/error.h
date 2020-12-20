#pragma once
#include <stdio.h>
#include <stdlib.h>

#define ERR -1
#define NO_ERR 0
#define EXEC_CMD_ERR 1
#define FORK_ERR 2
#define OPEN_DIR_ERR 3
#define CLOSE_DIR_ERR 4
#define OPEN_FILE_ERR 5
#define CLOSE_FILE_ERR 6
#define RECUP_STAT_ERR 7


#define syserror(n) perror(msgErr[n]),exit(n)


extern char* msgErr[];