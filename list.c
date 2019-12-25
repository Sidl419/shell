#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include "list.h"

#define SIZE 5

char ** lst;    // список слов (в виде массива)
char * buf;     // буфер для накопления текущего слова
int sizebuf;    // размер буфера текущего слова
int sizelist;   // размер списка слов
int curbuf;     // индекс текущего символа в буфере
int curlist;    // индекс текущего слова в списке

void nullbuf();
void null_list();
void clearlist();

void clearlist(){   // освобождает память, занимаемую списком
    int i;
    sizelist = 0;
    curlist = 0;

    if(lst == NULL) return;

    for(i = 0; lst[i] != NULL; i++)
        free(lst[i]);

    free(lst);
    lst = NULL;
}

void nullbuf(){     // присваивает буферу значение NULL
    buf = NULL;

    sizebuf = 0;
    curbuf = 0;
}

void null_list(){   // присваивает списку значение NULL
    sizelist = 0;
    curlist = 0;

    lst = NULL;
}

void termlist(){    // завершает список и обрезает занимаемую им память
    if(lst == NULL) return;

    if(curlist > sizelist - 1)
        lst = realloc(lst, (sizelist + 1) * sizeof(*lst));
        if(lst == NULL){
            printf("Memory error %d\n", errno);
            free(buf);
            nullbuf();
            null_list();
            return;
        }
    lst[curlist] = NULL;

    //выравниваем используемую под список память точно по размеру списка
    lst = realloc(lst, (sizelist = curlist + 1) * sizeof(*lst));
    if(lst == NULL){
        printf("Memory error %d\n", errno);
        free(buf);
        nullbuf();
        null_list();
        return;
    }
}

void addsym(int c){      // добавляет символ в текущее слово
    if(curbuf > sizebuf - 1){
        buf = realloc(buf, sizebuf += SIZE);    // увеличиваем буфер при необходимости
        if(buf == NULL){
            printf("Memory error %d\n", errno);
            nullbuf();
            clearlist();
            return;
        }
    }
    
    buf[curbuf++] = c;
}

void addword(){     // закрывает слово и добавляет его в список слов
    if(curbuf > sizebuf - 1){
        buf = realloc(buf, sizebuf += 1);   // для записи ’\0’ увеличиваем буфер при необходимости
        if(buf == NULL){
            printf("Memory error %d\n", errno);
            nullbuf();
            clearlist();
            return;
        }
    }
    buf[curbuf++] = '\0';

    buf = realloc(buf, sizebuf = curbuf);    //выравниваем используемую память точно по размеру слова
    if(buf == NULL){
        printf("Memory error %d\n", errno);
        nullbuf();
        clearlist();
        return;
    }

    if(curlist > sizelist - 1)
        lst = realloc(lst, (sizelist += SIZE) * sizeof(*lst));    // увеличиваем массив под список при необходимости
        if(lst == NULL){
            printf("Memory error %d\n", errno);
            free(buf);
            nullbuf();
            null_list();
            return;
        }
    lst[curlist++] = buf;
}

int symset(int c){  
// возвращает 1, если был считан любой символ, кроме пробела,
// табуляции, перевода строки и специальных символов, и не конец файла
    return (c != '\n') && 
           (c != ' ') && 
           (c != '\t') && 
           (c != '>') && 
           (c != '|') && 
           (c != '&') && 
           (c != '<') && 
           (c != '(') &&
           (c != ')') && 
           (c != ';') && 
           (c != EOF);
} 

void printlist(){   // печать списка слов
    int i;

    if(lst == NULL) return;

    printf("%d\n", sizelist - 1);   // печать размера списка
    for(i = 0; i < sizelist - 1; i++)   // печать элементов списка
        printf("%s ", lst[i]);
}

list exportlist(){
    if(lst == NULL) return NULL;

    list prev, exp = (list)malloc(sizeof(listnode));
    exp->word = lst[0];
    exp->next = NULL;
    prev = exp;

    for(int i = 1; i < sizelist - 1; i++){
        list temp = (list)malloc(sizeof(listnode));
        temp->word = lst[i];
        temp->next = NULL;
        prev->next = temp;
        prev = temp; 
    }

    return exp;
}

void printformat(list print){

    if(print == NULL) return;

    list temp = print;
    while(temp != NULL){   // печать элементов списка
        printf("%s ", temp->word);
        temp = temp->next;
    }
}

void clearformat(list * l){
    list prev, temp = *l;
    if(temp != NULL){
        while(temp != NULL){
            prev = temp;
            temp = temp->next;
            free(prev);
        }
        *l = NULL;
    }
}