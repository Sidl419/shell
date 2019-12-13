#ifndef LIST_H
#define LIST_H
#include <setjmp.h>

typedef struct List {
    char *word;
    struct List *next;
} list;

typedef enum {START, W, BRCK, SPEC, END} vertex2;

extern jmp_buf begin;

extern char ** lst;
extern char * buf;
extern int sizebuf;
extern int sizelist;
extern int curbuf;
extern int curlist;

void clearlist();
void null_list();
void termlist();
void nullbuf();
void addsym(int c);
void addword();
void printlist();
int symset(int c);
int lexcomp(char *a, char *b);
void sortlist();
char ** getlist(int *size);

#endif
