#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <pwd.h>
#include <alloca.h>
#include <time.h>
#include "./error.h"

#define PATH "/proc/"
#define STAT "/stat"
#define CMDLINE "/cmdline"
#define STATUS "/status"
#define UPTIME "/uptime"
#define CPUINFO "/cpuinfo"
#define MEMINFO "/meminfo"
#define HERTZ sysconf(_SC_CLK_TCK)
#define SIZE 20

typedef struct infos{
    char *user;
    int pid;

    long unsigned int utime;
    long unsigned int stime;
    unsigned cpu;

    unsigned long long mem;

    long unsigned int vsz;
    long int rss;

    char *tty;

    char stat;
    char nice;
    char pageslocked;
    char sessionleader;
    char multithread;
    char foreground;

    char *start;
    unsigned long long start_time;
    char *time;
    char *command;
} Infos;

DIR *direc;

void openProcDir(){
    if( !(direc = opendir(PATH)) ) syserror(3);
}

void closeProcDir(){
    if( closedir(direc) == ERR ) syserror(4);
}

void printAll(Infos *inf,int nb){
    printf("USER\tPID\t%%CPU\t%%MEM\tVSZ\tRSS\tTTY\tSTAT\tSTART\tTIME\tCOMMAND\n");
    for (int i=0; i < nb; i++) {
        printf("%-*s ",11,inf[i].user);
        printf("%*d ",5,inf[i].pid);
        if(inf[i].cpu > 999U){
            printf("%u  ",inf[i].cpu/10U);
        }
        else {
            printf("%-u.%u  ",inf[i].cpu/10U,inf[i].cpu%10U);
        }
        printf("%-u.%u ",(unsigned)(inf[i].mem/10),(unsigned)(inf[i].mem%10));
        printf("%*lu ",6,inf[i].vsz);
        printf("%*ld ",4,inf[i].rss);
        printf("- ");
        printf("%*c%c%c%c%c%c ",5,inf[i].stat,inf[i].nice,inf[i].pageslocked,inf[i].sessionleader,inf[i].multithread,inf[i].foreground);
        printf("%s ",inf[i].start);
        printf("- ");
        printf("%.*s ",40,inf[i].command);
        printf("\n");
    }

}

unsigned calcCpu(Infos *inf,float boot_time){
    unsigned long long temps_total;
    unsigned pcpu = 0;
    unsigned long long seconds;
    temps_total = inf->utime + inf->stime;

    seconds = (unsigned long long)boot_time >= (inf->start_time/HERTZ) ? (unsigned long long)boot_time - (inf->start_time/HERTZ):0;
    if(seconds) pcpu = (temps_total * 1000ULL / HERTZ) / seconds;
    return pcpu;
}

unsigned long long calcMem(Infos *inf, unsigned long long kb_total_mem){
    unsigned long pmem = 0;
    pmem = inf->rss * 1000ULL / kb_total_mem;
    if(pmem > 999) pmem = 999;
    return pmem;
}

void calc_start(Infos *inf,int btime){
    struct tm *temps_processus;
    struct tm *temps;
    time_t t;
    const char *format;
    size_t longueur;
    time_t seconds1970 = time(NULL);
    temps = localtime(&seconds1970);
    t = btime + inf->start_time / HERTZ;
    temps_processus = localtime(&t);
    format = "%H:%M";
    if(temps->tm_yday != temps_processus->tm_yday) format = "%b%d";
    if(temps->tm_year != temps_processus->tm_year) format = "%b%d";
    inf->start = malloc(sizeof(char)*240);
    longueur = strftime(inf->start, 240, format, temps_processus);
    if(longueur <=0 || longueur >= 240) inf->start[0] = '\0';
}


void boucleRead(){
    struct dirent *tmp_rep; // repertoire que je suis en train d ouvrir
    struct passwd *useruid;
    FILE *tmp_file; //fichier que je suis en train d ouvrir
    char path[270]; //chemin du fichier a ouvrir   (20 car /proc/ 5 carac max/ plusgrande chaine)
    char cmdchar;
    int i,j;
    float boot_time; // temps depuis le début du boot
    unsigned long long kb_total_mem; // valeur totale de ma ram en kb
    Infos *procinfos;
    int nbprocinfos,uid;

    long unsigned int poubelle;
    long unsigned int btime;
    int nice;
    int vmlck;
    int sessionleader;
    long int multithread;
    int pgrp;
    int tpgid;

    openProcDir();
    for (i = 0; (tmp_rep = readdir(direc));) {
        if(isdigit(tmp_rep->d_name[0]))
            i++;
    }
    nbprocinfos = i;
    procinfos = malloc(sizeof(Infos)*nbprocinfos);
    rewinddir(direc);

    sprintf(path, PATH""UPTIME); // Permet de recup time_boot
    if( !(tmp_file = fopen(path, "r")) ) syserror(5);
    fscanf(tmp_file, "%f",&boot_time); // recup boot_time
    if( (fclose(tmp_file)) == EOF ) syserror(6);

    sprintf(path, PATH""MEMINFO); // Permet de recup la valeur de ma ram
    if( !(tmp_file = fopen(path, "r")) ) syserror(5);
    fscanf(tmp_file, "%*[^:]: %lld",&kb_total_mem);
    if( (fclose(tmp_file)) == EOF ) syserror(6);

    sprintf(path, PATH""STAT); // Permet de recup btime
    if( !(tmp_file = fopen(path, "r")) ) syserror(5);
    for(int j=0;j < 11;j++)fscanf(tmp_file,"%*[^\n]\n");
    fscanf(tmp_file, "btime %lud",&btime); // Recup btime - 12
    if( (fclose(tmp_file)) == EOF ) syserror(6);

    for (i=0;(tmp_rep = readdir(direc)) ; ) {
        if (isdigit(tmp_rep->d_name[0])) {

            sprintf(path, PATH"%s"STAT,tmp_rep->d_name); // Permet de recup pid,command,stat
            if( !(tmp_file = fopen(path, "r")) ) syserror(5);

            procinfos[i].command = malloc(sizeof(char)*2048);
            fscanf(tmp_file, "%d %s %c",&procinfos[i].pid, procinfos[i].command, &procinfos[i].stat); // recup jusqu'au 3eme
            procinfos[i].command[0] = '[';
            procinfos[i].command[strlen(procinfos[i].command)-1] = ']';

            for (j=0;j<1;j++) fscanf(tmp_file, "%lu",&poubelle);
            fscanf(tmp_file, "%d",&pgrp); // permet de recup pgrp pour le s de STAT- 5
            fscanf(tmp_file, "%d",&sessionleader); // permet de recup session leader pour le s de STAT- 6
            if(sessionleader == procinfos[i].pid)procinfos[i].sessionleader='s';
            fscanf(tmp_file,"%lu",&poubelle);
            fscanf(tmp_file,"%d",&tpgid);
            if(pgrp == tpgid)procinfos[i].foreground='+';


            for (j=0; j<5; j++) fscanf(tmp_file,"%lu",&poubelle);
            fscanf(tmp_file, "%lu",&procinfos[i].utime); // permet de recup utime et stime pour calculer %cpu 14 et 15 indice
            fscanf(tmp_file, "%lu",&procinfos[i].stime);
            procinfos[i].cpu = calcCpu(&procinfos[i], boot_time);


            for (j=0; j<3; j++) fscanf(tmp_file,"%lu",&poubelle);
            fscanf(tmp_file, "%d",&nice); //nice 19
            if(nice > 0)procinfos[i].nice = 'N';
            if(nice < 0)procinfos[i].nice = '<';

            fscanf(tmp_file, "%ld",&multithread); //multithread 20
            if(multithread > 1)procinfos[i].multithread = 'l';

            for (j=0; j<1; j++) fscanf(tmp_file,"%lu",&poubelle);
            fscanf(tmp_file, "%llu",&procinfos[i].start_time); // temps depuis lequel le processus est lance - 22

            calc_start(&procinfos[i], btime);

            if( (fclose(tmp_file)) == EOF ) syserror(6);

            sprintf(path, PATH"%s"STATUS,tmp_rep->d_name);   // Permet de recup l'uid pour ensuite avoir user
            if( !(tmp_file = fopen(path, "r")) ) syserror(5);
            for (int j=0; j<8; j++) fscanf(tmp_file,"%*[^\n]\n");
            fscanf(tmp_file, "%*[^:]:\t%d",&uid); // recup de la 9 eme ligne
            if(uid <= -1) uid = 0;
            useruid = getpwuid(uid);
            procinfos[i].user = malloc(sizeof(char)*1024);
            strcpy(procinfos[i].user, useruid->pw_name);
            for (j=0; j<9; j++) fscanf(tmp_file,"%*[^\n]\n"); // va jusqu'a la ligne 18 (vsz)
            fscanf(tmp_file, "%*[^:]:\t%lu",&procinfos[i].vsz);
            fscanf(tmp_file,"%*[^\n]\n");
            fscanf(tmp_file, "%*[^:]:\t%d",&vmlck); //parse de VmLck - ligne 19
            if(vmlck != 0)procinfos[i].pageslocked='L'; //si la valeur est différente de 0 on ajoute un L a STAT du ps
            for (j=0; j<3; j++) fscanf(tmp_file,"%*[^\n]\n"); // va jusqu'a la ligne 22 (rss)
            fscanf(tmp_file, "%*[^:]:\t%ld",&procinfos[i].rss);
            if( (fclose(tmp_file)) == EOF ) syserror(6);

            procinfos[i].mem = calcMem(&procinfos[i],kb_total_mem); // Calcul du %MEM

            sprintf(path, PATH"%s"CMDLINE,tmp_rep->d_name);   // Permet de recup l'uid pour ensuite avoir user
            if( !(tmp_file = fopen(path, "r")) ) syserror(5);
            if( !( (cmdchar = fgetc(tmp_file)) == EOF)){
                j=0;
                do {
                if (cmdchar == '\0') cmdchar = ' ';
                procinfos[i].command[j++] = cmdchar;
                }while (!( (cmdchar = fgetc(tmp_file)) == EOF));
                procinfos[i].command[j-1] = '\0';
            }
            if( (fclose(tmp_file)) == EOF ) syserror(6);
            i++;
        }
    }
    closeProcDir();

    printAll(procinfos, nbprocinfos);

    for (int i=0; i<nbprocinfos; i++) {
        free(procinfos[i].command);
        free(procinfos[i].start);
        free(procinfos[i].user);
    }
    free(procinfos);
}


int main(void){
    boucleRead();
    return 0;
}


//getpwuid recup l user
