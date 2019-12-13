#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "list.h"
#include "tree.h"
#include "exec.h"
#include "formList.h"

#define BUFF_SIZE 10

jmp_buf begin;
list *plst;
intlist *bckgrnd;
int exit_val = 0;

void handler(int s)
{
    signal(SIGINT, handler);
}

void inv(){
    printf("%s", "\x1b[32m");  // здесь изменяется цвет на зеленый
    char s[100]; // ограничение: имя хоста и текущей директории не должно быть слишком длинным!
    gethostname(s, 100);
    printf("%s@%s", getenv("USER"), s);  
    printf("%s", "\x1B[37m");   // здесь изменяется цвет на серый
    getcwd(s, 100);
    printf(":%s$ ", s);
}

int main(){
    //inv();
    while(!end_of_file){
        printf("$ ");
        fflush(stdout);
        formList();
        printlist();
        clearlist();
        printf("\n");
    }
}

