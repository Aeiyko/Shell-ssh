#ifndef COLOR_LS
#define COLOR_LS

#define ROUGE(m) "\033[01;31m"m"\033[0m"
#define VERT(m) "\033[01;32m"m"\033[0m"
#define JAUNE(m) "\033[01;33m"m"\033[0m"
#define BLEU(m) "\033[01;34m"m"\033[0m"
#define MAGENTA(m) "\033[01;35m"m"\033[0m"
#define CYAN(m) "\033[01;36m"m"\033[0m"
#define BLANC(m) "\033[01;37m"m"\033[0m"

#define SURJAUN(m) "\033[30;43m"m"\033[0m"
#define SURROUGE(m) "\033[00;41m"m"\033[0m"
#define SURVERT(m) "\033[01;42m"m"\033[0m"
#define SUR(m)  "\033[01;43m"m"\033[0m"

#define COLOR(color, quotes, spaces) (quotes)?(color("\"%s\"")) : ((spaces)?(color("\'%s\'")):(color("%s")))

#endif
