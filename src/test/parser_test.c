#include "../commande/parser.h"

int main(void){
    char command_test[] = "ls -lra / &&cat /etc/pouet||pouet 2>>fichier&&cat & && cat $billy";
    print_test(command_test);
    return 0;
}