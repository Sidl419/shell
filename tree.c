#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "tree.h"

#define SIZE 5

list plex; /* указатель текущей лексемы, начальное значение передается через параметр функции
    build_tree(), список list – это массив указателей на лексемы-строки */
tree make_cmd(); /* создает дерево из одного элемента, обнуляет все поля */
void make_bgrnd(tree t); /* устанавливает поле backgrnd=1 во всех командах конвейера t */
void add_arg(); /* добавляет очередной элемент в массив argv текущей команды */
tree s; /* начальная команда, лучше назвать ее beg_cmd, это локальная переменная функции build_tree() */
    
tree c; /* текущая команда, лучше назвать ее cur_cmd, локальная переменная функции build_tree()*/
tree p; /* предыдущая команда, лучше назвать ее prev_cmd, тоже локальная */