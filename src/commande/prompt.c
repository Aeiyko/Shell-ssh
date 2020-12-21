#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CYAND "\033[01;36m"
#define CYANF "\033[0m"

int main(){
    char *path = get_current_dir_name();
    write(STDOUT_FILENO, CYAND, sizeof(char)*8);
    write(STDOUT_FILENO, path, sizeof(char)*strlen(path));
    write(STDOUT_FILENO, "> ", sizeof(char)*2);
    write(STDOUT_FILENO, CYANF, sizeof(char)*4);
    free(path);


    return 0;
}