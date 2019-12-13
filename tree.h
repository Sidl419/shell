#ifndef TREE_H
#define TREE_H

#include "list.h"

typedef enum {NXT, AND, OR} next_type;

typedef struct Tree {
    char **argv;
    char *infile;
    char *outfile;
    int backgrnd;
    next_type type;
    int append;
    struct Tree *psubcmd;
    struct Tree *pipe;
    struct Tree *next;
} tree;


extern jmp_buf begin;
extern list *plst;

void err_file();
void in_file(tree *);
void out_file(tree *);
void out_append(tree *); 
void error(char *, char *);
int is_oper();
int is_next();
int is_inout();
void background_sub(tree *);
void background(tree *);
void init_com(tree *);
tree *com_sh();
tree *com_list();
tree *conv();
tree *command();
tree *simple_com();
void print_struct(tree *);
void clear_tree(tree *);

#endif
