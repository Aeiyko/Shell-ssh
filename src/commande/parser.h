#pragma once

#define CH_LENGTH 1024

enum type{CMD,SEP,RED,VAR};
enum sep{ET,OU,PTV,PIPE,BACK};
enum red{STD,STDERR,BOTH,INJ};
enum redtype{SIMPLE,ADD};

static char *typestring[] = {
    "CMD",
    "SEP",
    "RED",
    "VAR"
};

static char *sepstring[] = {
    "ET",
    "OU",
    "PTV",
    "PIPE",
    "BACK"
};

static char *redstring[] = {
    "STD",
    "STDERR",
    "BOTH",
    "INJ"
};

static char *redtypestring[] = {
    "SIMPLE",
    "ADD"
};


struct parser{
    enum type type;
    enum sep sep;
    enum red red;
    enum redtype redtype;
    char command[CH_LENGTH];
};