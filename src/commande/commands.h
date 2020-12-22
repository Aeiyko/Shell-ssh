#define ERR -1
#define EXIT_NORMALLY 0
#define QUIT -2

#define BLOCK 1024
#define EXIT "exit"
#define SPACE " "

#define fatalerror(n) perror(msg[n-1]), exit(n);

#define IS_JOKER(c) (c=='?' || c=='*' || c=='[')

#define READ_ERR 1
#define WRITE_ERR 2
#define CMD_ERR 3

extern char* msg[];
int toArgs(char *command, char *args[BLOCK]);
int makeCd(char *cmd_tab[BLOCK], int nb_items);
int wildcard(char *cmd_tab[BLOCK], int *nb_items, int tab[BLOCK]);
int exec_cmd(char cmd[BLOCK]);
