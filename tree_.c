#include "tree_.h"

int end_flag = 0;
int is_error_tree = 0;

void make_shift(int n){
    while(n--)
        putc(' ', stderr);
}

void print_argv(list p, int shift){

    if(p == NULL) return;

    list temp = p;
    int count = 0;
    while(temp != NULL){   // печать элементов списка
        make_shift(shift);
        fprintf(stderr, "argv[%d]=%s\n",count, temp->word);
        ++count;
        temp = temp->next;
    }
}

void print_tree(tree t, int shift){
    list p;
    if(t == NULL)
        return;
    p = t->argv;
    make_shift(shift);
    fprintf(stderr, "argc=%d\n", t->argc);
    if(p != NULL)
        print_argv(p, shift);
    else{
        make_shift(shift);
        fprintf(stderr, "psubshell\n");
    }
    make_shift(shift);
    if(t->infile == NULL)
        fprintf(stderr, "infile=NULL\n");
    else
        fprintf(stderr, "infile=%s\n", t->infile);
    make_shift(shift);
    if(t->outfile == NULL)
        fprintf(stderr, "outfile=NULL\n");
    else
        fprintf(stderr, "outfile=%s\n", t->outfile);
    make_shift(shift);
    fprintf(stderr, "append=%d\n", t->append);
    make_shift(shift);
    fprintf(stderr, "background=%d\n", t->backgrnd);
    make_shift(shift);
    fprintf(stderr, "type=%s\n", t->type==NXT?"NXT": t->type==OR?"OR": "AND" );
    make_shift(shift);
    if(t->psubcmd == NULL)
        fprintf(stderr, "psubcmd=NULL \n");
    else{
        fprintf(stderr, "psubcmd---> \n");
        print_tree(t->psubcmd, shift+5);
    }
    make_shift(shift);
    if(t->pipe == NULL)
        fprintf(stderr, "pipe=NULL \n");
    else{
        fprintf(stderr, "pipe---> \n");
        print_tree(t->pipe, shift+5);
    }
    make_shift(shift);
    if(t->next == NULL)
        fprintf(stderr, "next=NULL \n");
    else{
        fprintf(stderr, "next---> \n");
        print_tree(t->next, shift+5);
    }
}

list plex; /* указатель текущей лексемы, начальное значение передается через параметр функции
build_tree(), список list – это массив указателей на лексемы-строки */

tree s; /* начальная команда, лучше назвать ее beg_cmd, это локальная переменная функции build_tree() */
tree c; /* текущая команда, лучше назвать ее cur_cmd, локальная переменная функции build_tree()*/
tree p; /* предыдущая команда, лучше назвать ее prev_cmd, тоже локальная */

tree make_cmd(){ /* создает дерево из одного элемента, обнуляет все поля */
    tree tr;
    tr = (tree)malloc(sizeof(treenode));

    tr->argv = NULL; // список из имени команды и аргументов
    tr->argc = 0;
    tr->infile = NULL; // переназначенный файл стандартного ввода
    tr->outfile = NULL; // переназначенный файл стандартного вывода
    tr->append = 0;
    tr->backgrnd = 0; // =1, если команда подлежит выполнению в фоновом режиме
    tr->psubcmd = NULL; // команды для запуска в дочернем shell
    tr->pipe = NULL; // следующая команда после "|"
    tr->next = NULL; // следующая после ";" (или после "&")
    tr->type = NXT; // связь со следующей командой через ; или && или ||

    return tr;
}

void make_bgrnd(tree t){ /* устанавливает поле backgrnd=1 во всех командах конвейера t */
    if(t == NULL){
        return;
    }
    t->backgrnd = 1;
    tree curt = t->pipe;
    while(curt != NULL){
        curt->backgrnd = 1;
        curt = curt->pipe;
    }
}

void add_arg(){ /* добавляет очередной элемент в массив argv текущей команды */
    list temp = (list)malloc(sizeof(listnode));
    temp->word = plex->word;
    temp->next = NULL;
    if(c->argv == NULL){
        c->argv = temp;
    }else{
        list cur = c->argv;
        while(cur->next != NULL){
            cur = cur->next;
        }
        cur->next = temp;
    }
    c->argc += 1;
}

typedef void * (*vertex1)();

void * begin(list);
void * conv(list);
void * conv1(list);
void * end();
void * backgrnd(list);
void * in(list);
void * out1(list);
void * out2(list);
void * errortree(list);
void * seq(list);

tree build_tree(list lst){
    if(lst == NULL){
        return NULL;
    }
    s = NULL;
    plex = lst;
    vertex1 V = begin(plex);
    while(!end_flag)
        V = V();
    return s;
}

int is_word(char* a){
    for(int i = 0; i < strlen(a); ++i){
        switch(a[i]){
            case 'a'...'z':
                break;
            case 'A'...'Z':
                break;
            case '-':
                break;
            case '_':
                break;
            case '/':
                break;
            case '0'...'9':
                break;
            case ' ':
                break;
            case '\\':
                break;
            case '(':
                break;
            case ')':
                break;
            case '.':
                break;
            case '\n':
                break;
            case '\'':
                break;
            case '\"':
                break;
            default:
                return 0;
        }
    }
    return 1;
} 

void* begin(list cur){
    plex = cur;
    if(is_word(cur->word)){
        c = make_cmd();
        s = c;
        add_arg();
        p = c;
        return conv(cur->next);
    }
    return errortree(cur);
}

void* seq(list cur){
    plex = cur;
    if(is_word(cur->word)){
        c = make_cmd();
        add_arg();
        p->next = c;
        p->type = NXT;
        p = c;
        return conv(cur->next);
    }
    return errortree(cur);
}

void* conv(list cur){
    plex = cur;
    if(cur == NULL){
        return end;
    }
    if(strlen(cur->word) == 1){
        if(cur->word[0] == '|'){
            return conv1(cur->next);
        }
        if(cur->word[0] == '&'){
            return backgrnd(cur->next);
        }
        if(cur->word[0] == ';'){
            return seq(cur->next);
        }
        if(cur->word[0] == '<'){
            return in(cur->next);
        }
        if(cur->word[0] == '>'){
            return out1(cur->next);
        }
    }
    if(strlen(cur->word) == 2 && cur->word[0] == '>' && cur->word[1] == '>'){
        return out2(cur->next);
    }
    if(is_word(cur->word)){
        add_arg();
        return conv(cur->next);
    }
    return errortree(cur);
}

void* in(list cur){
    plex = cur;
    if(is_word(cur->word)){
        c->infile = cur->word;
        return conv(cur->next);
    }
    return errortree(cur);
}

void* out1(list cur){
    plex = cur;
    if(is_word(cur->word)){
        c->outfile = cur->word;
        return conv(cur->next);
    }
    return errortree(cur);
}

void* out2(list cur){
    plex = cur;
    if(is_word(cur->word)){
        if(c->outfile == NULL)
            c->outfile = cur->word;
        c->append = 1;
        return conv(cur->next);
    }
    return errortree(cur);
}

void* backgrnd(list cur){
    plex = cur;
    if(cur == NULL){
        make_bgrnd(s);
        return end();
    }
    return errortree(cur);
}

void* conv1(list cur){
    plex = cur;
    if(is_word(cur->word)){
        c = make_cmd();
        add_arg();
        p->pipe = c;
        p = c;
        return conv(cur->next);
    }
    return errortree(cur);
}

void * errortree(list cur){
    printf("Syntax error near %s\n", cur->word);
    is_error_tree = 1;
    return end;
}

void* end(){
    end_flag = 1;
    return begin;
}

void clear_tree(tree *tr){
    tree tempt = *tr; 
    if(tempt == NULL){
        return;
    } 
    if(tempt->argv != NULL){
        list prev, temp = tempt->argv;
        while(temp != NULL){
            prev = temp;
            temp = temp->next;
            free(prev);
        }
        tempt->argv = NULL;
    }
    clear_tree(&(tempt->pipe));
    clear_tree(&(tempt->next));
    free(tempt);
    *tr = NULL;
}