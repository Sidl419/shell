#include "list.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "formList.h"
#include <setjmp.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>

#define N 16    // размер буфера ввода

typedef void * (*vertex1)();

int c;          // текущий символ
char str[N];
int curstr = N;
int end_of_file = 0;

static jmp_buf jmp_stop;

void * start();
void * word();
void * greater();
void * greater2();
void * newline();
void * stop();
void * or();
void * or2();
void * and2();
void * and();
void * semicolon();
void * less();
void * lbracket();
void * rbracket();
void * error();
void * quotation();
void * slash();
void * hash();
void * variable();

int symcnt = N;     // фактическое количество символов, считанных в буфер

int getsym(){   // возвращает символ из входного потока
	if(curstr == symcnt){   // читаем новую порцию данных, если текущая подошла к концу
		symcnt = read(0, str, N * sizeof(str[0]));
        curstr = 0;
    }
	return (symcnt == 0 ? EOF : (int)str[curstr++]);//  возвращаем текущий символ или EOF
}

void formList(){
    vertex1 V = start;
    c = getsym();
    null_list();
    while(!setjmp(jmp_stop))
        V = V();
}

void* start(){
    if(c == ' ' || c == '\t'){
        c = getsym(); 
        return start;
    }
    else if(c == EOF){
        termlist();
        end_of_file = 1;
        return stop;
    }
    else if(c == '\n'){
        termlist();
        return stop;
    }
    else{
        char cprev = c;
        nullbuf();
        if(cprev == '\"'){
            return quotation;
        }
        if(cprev == '\\'){
            return slash;
        }
        if(cprev == '#'){
            return hash;
        }
        if(cprev == '$'){
            return variable;
        }
        addsym(c);
        c = getsym();
        switch(cprev){
            case '>':
                return greater;
            case '|':
                return or;
            case '&':
                return and;
            case ';':
                return semicolon;
            case '<':
                return less;
            case '(':
                return lbracket;
            case ')':
                return rbracket;
            case '0' ... '9':
                return word;
            case 'A' ... 'Z':
                return word;
            case 'a' ... 'z':
                return word;
            case '_':
                return word;
            case '/':
                return word;
            case '.':
                return word;
            default:
                return word;
        }
    }
}

void* word(){
    if(symset(c)){
        switch(c){
            case '0' ... '9':
                break;
            case 'A' ... 'Z':
                break;
            case 'a' ... 'z':
                break;
            case '$':
                return variable;
            case '_':
                break;
            case '/':
                break;
            case '.':
                break;
            case ';':
                break;
            case '\"':
                return quotation;
            case '\\':
                return slash;
            default:
                break;
        }
        addsym(c);
        c = getsym();
        return word;
    }
    else{
        addword();
        return start;
    }
}

void* variable(){
    char s[5];
    s[4] = ' ';
    char buf1[PATH_MAX];
    char* buf;
    int i = 0;
    c = getsym();
    while(c <= 'Z' && c >= 'A'){
        s[i] = c;
        ++i;
        c = getsym();
    }
    
    if(strcmp(s, "SHELL"))
        buf = getcwd(buf1, PATH_MAX);
    else
        if(strcmp(s, "HOME "))
            buf = getenv("HOME");
        else
            if(strcmp(s, "EUID "))
                buf = geteuid();
            else
                if(strcmp(s, "USER "))
                    buf = getlogin();
                else
                {
                    /* code */
                }
    
    for(int j = 0; j < strlen(buf); ++j){
        c = s[j];
        addsym(c);
    }

    c = getsym();

    return word;
}

void* hash(){
    while(c != '\n' && c != EOF){
        c = getsym();
    }

    if(c == EOF)
        end_of_file = 1;

    termlist();
    return stop;
}

void* slash(){
    c = getsym();
    addsym(c);
    c = getsym();
    return word;
}

void* quotation(){
    c = getsym();

    while(c != '\"'){
        addsym(c);
        c = getsym();
    }

    c = getsym();
    return word;
}

void* greater(){
    if(c == '>'){
        addsym(c);
        c = getsym();
        return greater2;
    }
    else{
        addword();
        return start;
    }
}

void* semicolon(){
    addword();
    return start;
}

void* less(){
    addword();
    return start;
}

void* lbracket(){
    addword();
    return start;
}

void* rbracket(){
    addword();
    return start;
}

void* greater2(){
    addword();
    return start;
}

void* or(){
    if(c == '|'){
        addsym(c);
        c = getsym();
        return or2;
    }
    else{
        addword();
        return start;
    }
}

void* or2(){
    addword();
    return start;
}

void* and(){
    if(c == '&'){
        addsym(c);
        c = getsym();
        return and2;
    }
    else{
        addword();
        return start;
    }
}

void* and2(){
    addword();
    return start;
}

void* stop(){
    longjmp(jmp_stop, 1); 
} 