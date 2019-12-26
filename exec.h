#ifndef EXEC_H
#define EXEC_H

#include "tree_.h"

typedef struct backgrndList {
    int pid;
    struct backgrndList *next;
} intlistnode;

typedef intlistnode* intlist;

extern intlist *bckgrnd;
extern jmp_buf begin1;
extern int exit_val;
extern int error_in_com;
extern int cur_working_proc;

void add_elem (intlist *, int);
void print_intlist(intlist);
int clear_intlist(int *, intlistnode *);
void clear_zombie(intlist *);
void fullclearpid(intlist *);
//int is_com(tree);
int exec_cd(tree);
int exec_pwd(tree, int);
int exec_exit(tree);
//int exec_clear();
//int sh_com(tree);
void chng_iofiles(int, int, int, tree);
int exec_com_sh(tree);
int exec_com_list(tree, int);
int exec_conv(tree, int, int *);
//int exec_command(tree, int, int, int, int , intlist *);
int exec_simple_com(tree, int, int, int, int *);

#endif
