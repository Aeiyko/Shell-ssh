#include <stdio.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>
#include "../myssh-server/memoire.h"

int main(){
    char *pouet;
    create_mem();
    set_mem("billy=pouet");
    set_mem("Michel=pouet2");
    set_mem("Natacha=pouet3");
    set_mem("Frederic=pouet4");
    set_mem("Christophe=pouet5");
    pouet = get_mem("billy");
    printf("test : %s\n",pouet);
    pouet = get_mem("Natacha");
    printf("test : %s\n",pouet);
    pouet = get_mem("Michel");
    printf("test : %s\n",pouet);
    pouet = get_mem("Christophe");
    printf("test : %s\n",pouet);
    pouet = get_mem("Frederic");
    printf("test : %s\n",pouet);
    unset_mem("Natacha");
    printf("poe\n");
    if(!get_mem("Natacha"))printf("MORT\n");
    destroy_mem();
    return 0;
}