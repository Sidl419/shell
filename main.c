#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "list.h"
#include "exec.h"
#include "formList.h"
#include "tree_.h"

#define BUFF_SIZE 10

jmp_buf begin1;
list *plst;
intlist *bckgrnd;
int exit_val;
tree tr;
list lexlist = NULL;

void handler(int s){
    clear_tree(&tr);
    clearformat(lexlist);
    clearlist();
    printf("\n");
    longjmp(begin1, 1); 
}

void inv(){
    printf("%s", "\x1b[32m");  // здесь изменяется цвет на зеленый
    char s[100]; // ограничение: имя хоста и текущей директории не должно быть слишком длинным!
    gethostname(s, 100);
    printf("%s@%s", getenv("USER"), s);  
    printf("%s", "\x1B[37m");   // здесь изменяется цвет на серый
    getcwd(s, 100);
    printf(":%s$ ", s);
    fflush(stdout);
}

int main(){
    signal(SIGINT, handler);
    setjmp(begin1);
    while(!end_of_file){
        inv();
        fflush(stdout);
        formList();
        clear_zombie(bckgrnd);
        if(is_error_list){
            clearlist();
            is_error_list = 0;
            continue;
        }

        
        //printlist();
        //printf("\n");
        lexlist = exportlist();
        //printformat(lexlist);
        //print("\n");
        
        tr = NULL;
        tr = build_tree(lexlist);
        if(is_error_tree){
            clear_tree(&tr);
            clearformat(lexlist);
            clearlist();
            is_error_tree = 0;
            continue;
        }
        //print_tree(tr, 2);

        exit_val = exec_com_sh(tr);
        
        clear_tree(&tr);
        clearformat(lexlist);
        clearlist();
    }
    clear_zombie(bckgrnd);
}