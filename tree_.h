#ifndef TREE_H
#define TREE_H

#include "list.h"
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <stdio.h>

enum type_of_next{
    NXT, AND, OR   // виды связей соседних команд в списке команд
};

struct cmd_inf {
    list argv; // список из имени команды и аргументов
    int argc;
    char *infile; // переназначенный файл стандартного ввода
    char *outfile; // переназначенный файл стандартного вывода
    int append;  // =1, если вывод дописывается в конец файла
    int backgrnd; // =1, если команда подлежит выполнению в фоновом режиме
    struct cmd_inf* psubcmd; // команды для запуска в дочернем shell
    struct cmd_inf* pipe; // следующая команда после "|"
    struct cmd_inf* next; // следующая после ";" (или после "&")
    enum type_of_next type;// связь со следующей командой через ; или && или ||
};

extern int is_error_tree;

typedef struct cmd_inf *tree;
typedef struct cmd_inf treenode;

void print_tree(tree, int);
tree build_tree(list);
void clear_tree(tree *);

#endif