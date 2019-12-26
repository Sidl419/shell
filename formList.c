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
int is_error_list = 0;
int is_sigint;

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
void * quotation1();
void * quotation2();
void * slash();
void * hash();
void * variable();
void * errorlist();

void reverse(char s[]){
    int i, j;
    char c;
 
    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void myitoa(int n, char s[]){
    int i, sign;
 
    if ((sign = n) < 0)  /* записываем знак */
        n = -n;          /* делаем n положительным числом */
    i = 0;
    do {       /* генерируем цифры в обратном порядке */
        s[i++] = n % 10 + '0';   /* берем следующую цифру */
    }while ((n /= 10) > 0);     /* удаляем */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

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
    if(is_sigint){ 
        c = getsym();
        is_sigint = 0;
    }
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
            return quotation2;
        }
        if(cprev == '\''){
            return quotation1;
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
                return quotation2;
            case '\'':
                return quotation1;
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
    char s[6];
    char* buf = NULL;
    int i = 0, f = 0;

    c = getsym();
    while(c <= 'Z' && c >= 'A' && i < 5){
        s[i] = c;
        if(i == 3 && strncmp(s, "SHEL", 4)){
            c = getsym();
            break;
        }
        ++i;
        c = getsym();
    }
    
    if(!strcmp(s, "SHELL"))
        buf = getenv("SHELL");
    else{
        s[4] = ' ';
        if(!strcmp(s, "HOME "))
            buf = getenv("HOME");
        else
            if(!strcmp(s, "EUID ")){
                int n = 0, a, b = geteuid();
                a = b;
                while(b != 0){
                    b /= 10;
                    ++n;
                }
                buf = (char*)malloc((n + 1) * sizeof(char));
                myitoa(a, buf);
                f = 1;
            }else
                if(!strcmp(s, "USER "))
                    buf = getlogin();
                else
                {
                    printf("Variable(s) does not exist\n");
                    return errorlist;
                }
    }

    char a;
    
    for(int j = 0; j < strlen(buf); ++j){
        a = buf[j];
        addsym(a);
    }

    if(f) free(buf);

    if(c == '\n' || c == EOF){
        addword();
        return start;
    }else{
        return word;
    }
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

void* quotation2(){
    c = getsym();
    if(c == '\\'){
        char p = c;
        c = getsym();
        if(c != '\"'){
            addsym(p);
        }
        addsym(c);
        c = getsym();
    }

    while(c != '\"'){
        addsym(c);
        c = getsym();
        if(c == '\\'){
            char p = c;
            c = getsym();
            if(c != '\"'){
                addsym(p);
            }
            addsym(c);
            c = getsym();
        }
    }

    c = getsym();
    return word;
}

void* quotation1(){
    c = getsym();
    if(c == '\\'){
        char p = c;
        c = getsym();
        if(c != '\''){
            addsym(p);
        }
        addsym(c);
        c = getsym();
    }

    while(c != '\''){
        addsym(c);
        c = getsym();
        if(c == '\\'){
            char p = c;
            c = getsym();
            if(c != '\''){
                addsym(p);
            }
            addsym(c);
            c = getsym();
        }
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

void* errorlist(){
    is_error_list = 1;
    addword();
    termlist();
    while((c != '\n') && (c != EOF))
        c = getsym();
    clearlist();
    return stop;
}