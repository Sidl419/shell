#ifndef LIST_H
#define LIST_H
#include <setjmp.h>

typedef struct ListNode {
    char *word;
    struct ListNode *next;
} listnode;

typedef listnode *list;

typedef enum {START, W, BRCK, SPEC, END} vertex2;

extern jmp_buf begin1;

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
list exportlist();
void printformat(list);
void clearformat(list *);

#endif
