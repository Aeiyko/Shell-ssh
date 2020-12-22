#pragma once

#define CH_LENGTH 1024

enum type{CMD,SEP,RED,VAR};
enum sep{ET,OU,PTV,PIPE,BACK};
enum red{STD,STDERR,BOTH,INJ};
enum redtype{SIMPLE,ADD};

struct parser{
    enum type type;
    enum sep sep;
    enum red red;
    enum redtype redtype;
    char command[CH_LENGTH];
};

void print_test(char *cmd);
int parser(char *cmd,struct parser parser[CH_LENGTH]);