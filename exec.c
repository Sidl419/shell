#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>
#include "exec.h"
#include "formList.h"

int error_in_com = 0;
int size = 0;
intlist intlst = NULL;
intlist *bckgrnd = &intlst;

int exec_cd(tree tr){
    char *buf;
    if(tr->argc == 1){
        buf = getenv("HOME");
    }else{
        buf = tr->argv->next->word;
    }
    if(chdir(buf) != 0){
        perror("Error. Can't change the directory.");
        return 1;
    }
    return 0;
}

int exec_pwd(tree tr, int out){
    char buf[PATH_MAX];
    if(getcwd(buf, PATH_MAX) == NULL){
        perror("Error. Your path is too long.");
        return 1;
    }else{
        if(tr->outfile != NULL){
            if(tr->append)
                out = open(tr->outfile, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 0666);
            else
                out = open(tr->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        }
        write(out, buf, strlen(buf) * sizeof(char));
        printf("\n");
        if(tr->outfile != NULL)
            close(out);
    }
    return 0;
}

int exec_exit(tree tr){
    end_of_file = 1;
    return 0;
}

void add_elem (intlist * lst, int elem){
    if(lst == NULL) return;
    intlist temp = (intlist)malloc(sizeof(intlistnode));
    //int size = 1;
    temp->pid = elem;
    temp->next = NULL;
    if(*lst == NULL){
        *lst = temp;
    }else{
        intlist cur = *lst;
        while(cur->next != NULL){
            //++size;
            cur = cur->next;
        }
        cur->next = temp;
    }
    //printf("[%d] %d\n", size, elem);
}

void print_intlist(intlist lst){
    if(lst == NULL) return;

    intlist temp = lst;
    while(temp != NULL){   // печать элементов списка
        printf("%d ", temp->pid);
        temp = temp->next;
    }
    printf("\n");
}

int clear_intlist(intlistnode * node){
    if(node == NULL) return 1;

    int status;
    if(waitpid(node->pid, &status, WNOHANG) != 0){
        size--;
        free(node);
        node = NULL;
        return 1;
    }else 
        return 0;
}

void clear_zombie(intlist * lst){
    if(lst == NULL) return;
    int size = 1, is_first = 1;
    if(*lst != NULL){
        intlist prev, next, cur = *lst;
        while(cur != NULL){
            next = cur->next;
            if(!clear_intlist(cur)){
                if(is_first){
                    lst = &cur;
                    is_first = 0;
                }else{
                    prev->next = cur;
                }
                prev = cur;
                cur = next;
                prev->next = NULL;
            }else{
                cur = next;
                printf("[%d] Done\n", size);
            }
            ++size;
        }
        if(is_first){
            *lst = NULL;
        }
    }
}

void chng_iofiles(int is_pipe, int in_pipe, int out_pipe, tree tr){
    if(tr == NULL) return;

    if(is_pipe == 1){
        close(in_pipe);
        dup2(out_pipe,1);
        close(out_pipe);
    }else if(is_pipe == 2){
        dup2(in_pipe,0);
        close(in_pipe);
        dup2(out_pipe,1);
        close(out_pipe);
    } else if(is_pipe == 3){
        dup2(in_pipe,0);
        close(in_pipe);
    }

    if(tr->infile != NULL){
        int inf = open(tr->infile, O_RDONLY, 0666);
        dup2(inf, 0);
        close(inf);
    }

    if(tr->outfile != NULL){
        int outf;
        if(tr->append)
            outf = open(tr->outfile, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 0666);
        else
            outf = open(tr->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        dup2(outf, 1);
        close(outf);
    }
}

int exec_simple_com(tree tr, int in_pipe, int out_pipe, int is_pipe, int * pid, intlist * lst){
    if(tr == NULL) return -1;
    char *prog_name = tr->argv->word;
    int status, ch_pid;
    if(!strcmp(prog_name, "cd")){
        if(tr->backgrnd == 0){
            if(is_pipe)
                *pid = 0;
            return 0;
        }else{
            if(!(ch_pid = fork())){
                close(0);
                signal(SIGINT, SIG_IGN);
                int ch_res = exec_cd(tr);
                exit(ch_res);
            }
            add_elem(lst, ch_pid);
            return 0;
        }
    }
    if(!strcmp(prog_name, "pwd")){
        int out_desc = 1;
        if(is_pipe == 1 || is_pipe == 2){
            out_desc = out_pipe;
        }
        if(tr->backgrnd == 0){
            if(is_pipe)
                *pid = 0;
            return exec_pwd(tr, out_desc);
        }else{
            if(!(ch_pid = fork())){
                close(0);
                signal(SIGINT, SIG_IGN);
                int ch_res = exec_pwd(tr, out_desc);
                exit(ch_res);
            }
            add_elem(lst, ch_pid);
            return 0;
        }
    }
    if(!strcmp(prog_name, "exit")){
        if(tr->backgrnd == 0){
            if(is_pipe)
                *pid = 0;
            return 0;
        }else{
            if(!(ch_pid = fork())){
                close(0);
                signal(SIGINT, SIG_IGN);
                int ch_res = exec_exit(tr);
                exit(ch_res);
            }
            add_elem(lst, ch_pid);
            return 0;
        }
    }
    if(!(ch_pid = fork())){
        if(tr->backgrnd){
            close(0);
            signal(SIGINT, SIG_IGN);
        }

        if(is_pipe){

        }
        chng_iofiles(is_pipe, in_pipe, out_pipe, tr);

        char * count_args[tr->argc + 1];
        list args = tr->argv;
        for(int i = 0; i < tr->argc; ++i){
            count_args[i] = args->word;
            args = args->next;
        }
        count_args[tr->argc] = NULL;
        execvp(count_args[0], count_args);
        printf("Error in command: %s \n", count_args[0]);
        exit(1);
    }
    if(tr->backgrnd == 0){
        if(is_pipe){
            *pid = ch_pid;
            return 0;
        }
        waitpid(ch_pid, &status, 0);
        return WEXITSTATUS(status);
    }else{
        *pid = ch_pid;
        add_elem(lst, ch_pid);
        return 0;
    }
}

int exec_conv(tree tr, int len){
    if(tr == NULL) return -1;

    int back = tr->backgrnd;
    int res, fd[2];
    int wait_pid = 0;

    if(len == 1){
        res = exec_simple_com(tr, 0, 0, 0, &wait_pid, bckgrnd);
        if(back)
            printf("[%d] %d\n", ++size, wait_pid);
        return res;
    }

    tree temp = tr;
    
    int* pid_list = (int*)malloc(len * sizeof(int));

    pipe(fd);
    int out = fd[1], next_in = fd[0];
    
    res = exec_simple_com(temp, next_in, out, 1, &wait_pid, bckgrnd);
    int in = next_in;
    pid_list[0] = wait_pid;
    temp = temp->pipe;

    if(back){
        printf("[%d] %d\n", ++size, wait_pid);
    }

    for(int i = 1; i < len - 1; ++i){
        close(out);
        pipe(fd);
        out = fd[1];
        next_in = fd[0];

        res = exec_simple_com(temp, in, out, 2, &wait_pid, bckgrnd);
        pid_list[i] = wait_pid;
        temp = temp->pipe;
        close(in);
        in = next_in;
    }

    close(out);
    res = exec_simple_com(temp, in, 1, 3, &wait_pid, bckgrnd);
    pid_list[len - 1] = wait_pid;
    close(in);

    int status;
    if(!back){
        for(int i = 0; i < len; ++i){
            if(pid_list[i] != 0){
                waitpid(pid_list[i], &status, 0);
            }
        }
    }

    free(pid_list);
    

    if(!back)
        return WEXITSTATUS(status);
    else
        return 0;
}

int len_conv(tree tr){
    if(tr == NULL) return -1;

    int len = 1;
    tree temp = tr;
    while(temp->pipe != NULL){
        temp = temp->pipe;
        ++len;
    } 

    return len;
}

int exec_com_list(tree tr, int len){
    if(tr == NULL) return -1;

    int res;
    tree temp = tr;

    for(int i = 0; i < len; ++i){
        res = exec_conv(temp, len_conv(temp));
        temp = temp->next;
    }
    
    return res;
}

int exec_com_sh(tree tr){
    if(tr == NULL) return -1;

    int res, len = 1;
    tree temp = tr;
    while(temp->next != NULL){
        temp = temp->next;
        ++len;
    }

    res = exec_com_list(tr, len);
    return res;
}