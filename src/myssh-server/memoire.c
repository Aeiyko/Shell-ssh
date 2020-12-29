#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include "memoire.h"

#define SHM_SIZE 10000

static key_t g_key = 4242;
static void *g_memoire;
static int g_shmid;
static int g_semid;
static size_t g_indice;

struct sembuf p={0,-1,SEM_UNDO};
struct sembuf v={0,+1,SEM_UNDO};

void calcul_indice(){
    for(;strlen((const char *)g_memoire+g_indice) !=0;)g_indice += strlen((const char *)g_memoire+g_indice)+1;
}


void create_mem(){
    g_indice = 0;
    if( (g_shmid = shmget(g_key, SHM_SIZE, 0600 | IPC_CREAT | IPC_EXCL)) == -1){
        g_shmid = shmget(g_key, SHM_SIZE, 0600 | IPC_CREAT);
        calcul_indice();
    }
    else {
        g_semid = semget(g_key, 1, 0600 | IPC_CREAT);
        semctl(g_semid, 0, SETVAL, 0);
    }
    g_memoire = shmat(g_shmid, NULL, 0);
    semop(g_shmid, &v, 1);
}

void set_mem(char *valeur){
    memcpy(g_memoire+g_indice, valeur, strlen(valeur));
    g_indice += strlen(valeur)+1; 
}

char *get_mem(char *valeur){
    int i=0;
    char *name;
    char *val;

    for (int i =0;i != g_indice;i+=strlen(g_memoire+i)+1) {
        name = strtok(g_memoire+i, "=");
        val = strtok(NULL, "=");
        if (!strcmp(valeur,name)) {
            name[strlen(name)] = '=';
            return val;
        }
        name[strlen(name)] = '=';
    }
    return NULL;
}

void unset_mem(char *valeur){
    int i=0;
    char *val;
    char *valsuiv;
    int lenval;
    size_t quantiterestante = 0;
    int tmp;

    for (int i =0;i != g_indice;i+=strlen(g_memoire+i)+1) {
        val = strtok(g_memoire+i, "=");
        if (!strcmp(valeur,val)) {
            val[strlen(val)] = '=';
            break;
        }
        val[strlen(val)] = '=';
    }

    lenval = strlen(val);
    memset(val, 0, lenval);
    valsuiv = val+lenval+1;
    for(;(tmp=strlen(valsuiv+quantiterestante));quantiterestante+=tmp+1);
    memmove(val, valsuiv, quantiterestante);
    g_indice -= lenval+1;
}

void destroy_mem(){
    int valeur_sem = semctl(g_semid, 0, GETVAL);
    semop(g_shmid, &p, 1);
    shmdt(g_memoire);
    if(valeur_sem == -1)perror("error semctl"),exit(1);
    else if( valeur_sem == 0 ){
        shmctl(g_shmid, IPC_RMID, NULL);
        semctl(g_semid, 0,IPC_RMID);
    }
}
