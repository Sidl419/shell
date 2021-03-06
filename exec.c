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
int cur_working_proc;
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
    int size = 1;
    temp->pid = elem;
    temp->next = NULL;
    if(*lst == NULL){
        *lst = temp;
    }else{
        intlist cur = *lst;
        while(cur->next != NULL){
            ++size;
            cur = cur->next;
        }
        cur->next = temp;
    }
    printf("[%d] %d\n", size, elem);
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

int clear_intlist(int * info, intlistnode * node){
    if(node == NULL) return 1;

    int status;
    *info = 0;
    if(waitpid(node->pid, &status, WNOHANG) != 0){
        size--;
        free(node);
        node = NULL;
        *info = status;
        return 1;
    }else 
        return 0;
}

void clear_zombie(intlist * lst){
    if(lst == NULL) return;
    int status, size = 1, is_first = 1;
    if(*lst != NULL){
        intlist prev, next, cur = *lst;
        while(cur != NULL){
            next = cur->next;
            if(!clear_intlist(&status, cur)){
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
                printf("[%d] Exit code: %d\n", size, WEXITSTATUS(status));
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
            outf = open(tr->outfile, O_WRONLY | O_APPEND | O_CREAT, 0666);
        else
            outf = open(tr->outfile, O_WRONLY | O_CREAT, 0666);
        dup2(outf, 1);
        close(outf);
    }
}

int exec_simple_com(tree tr, int in_pipe, int out_pipe, int is_pipe, int * pid){
    if(tr == NULL) return -1;

    char *prog_name = tr->argv->word;
    int status, ch_pid;

    if(!strcmp(prog_name, "cd")){
        if(is_pipe){
            *pid = 1;
            return 0;
        }else
            return exec_cd(tr);
    }

    if(!strcmp(prog_name, "pwd")){
        int out_desc = 1;
        if(is_pipe == 1 || is_pipe == 2){
            out_desc = out_pipe;
        }
        if(is_pipe)
            *pid = 1;
        return exec_pwd(tr, out_desc);
    }

    if(!strcmp(prog_name, "exit")){
        if(is_pipe){
            *pid = 1;
            return 0;
        }else
            return exec_exit(tr);
    }

    if(!(ch_pid = fork())){

        if(!(tr->backgrnd)){
            cur_working_proc = ch_pid;
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
        perror(count_args[0]);
        exit(1);
    }

    if(is_pipe){
        *pid = 0;
        return 0;
    }
    waitpid(ch_pid, &status, 0);
    if(!(tr->backgrnd))
        cur_working_proc = 0;
    return WEXITSTATUS(status);
}

int exec_conv(tree tr, int len, int *stat){
    if(tr == NULL) return -1;

    int ch_pid, if_wait = 0;

    if(len == 1){
        char *prog_name = tr->argv->word;
        if(!strcmp(prog_name, "cd"))
            return exec_cd(tr);
        if(!strcmp(prog_name, "pwd"))
            return exec_pwd(tr, 1);
        if(!strcmp(prog_name, "exit"))
            return exec_exit(tr);
    }

    if(!(ch_pid = fork())){
        int res, fd[2];

        if(len == 1){
            res = exec_simple_com(tr, 0, 0, 0, &if_wait);
            exit(res);
        }

        tree temp = tr;
        
        int pid_count = 0;

        pipe(fd);
        int out = fd[1], next_in = fd[0];
        
        res = exec_simple_com(temp, next_in, out, 1, &if_wait);
        int in = next_in;
        pid_count += if_wait;
        temp = temp->pipe;

        for(int i = 1; i < len - 1; ++i){
            close(out);
            pipe(fd);
            out = fd[1];
            next_in = fd[0];

            res = exec_simple_com(temp, in, out, 2, &if_wait);
            pid_count += if_wait;
            temp = temp->pipe;
            close(in);
            in = next_in;
        }

        close(out);
        res = exec_simple_com(temp, in, 1, 3, &if_wait);
        pid_count += if_wait;
        close(in);

        for(int i = 0; i < len - pid_count; ++i){
            waitpid(0, NULL, 0);
        }

        exit(res);
    }

    int status;
    waitpid(ch_pid, &status, 0);
    *stat = status;
    return WEXITSTATUS(status);
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

int exec_com_seq(tree tr){
    if(tr == NULL) return -1;

    int back = tr->backgrnd;

    if(!back){

        int res, inf;
        tree temp = tr;

        res = exec_conv(temp, len_conv(temp), &inf);

        while(temp->type != NXT && temp->next != NULL){
            if(temp->type == AND && !WIFSIGNALED(inf) && WIFEXITED(inf) && (WEXITSTATUS(inf) == 0)){
                temp = temp->next;
                res = exec_conv(temp, len_conv(temp), &inf);
            }
            if(temp->type == OR && !WIFSIGNALED(inf) && WIFEXITED(inf) && (WEXITSTATUS(inf) != 0)){
                temp = temp->next;
                res = exec_conv(temp, len_conv(temp), &inf);
            }
            break;
        }
        return res;
    }else{
        int ch_pid;

        if(!(ch_pid = fork())){
        
            close(0);
            signal(SIGINT, SIG_IGN);

            int res, inf;
            tree temp = tr;

            res = exec_conv(temp, len_conv(temp), &inf);

            while(temp->type != NXT && temp->next != NULL){
                if(temp->type == AND && !WIFSIGNALED(inf) && WIFEXITED(inf) && (WEXITSTATUS(inf) == 0)){
                    temp = temp->next;
                    res = exec_conv(temp, len_conv(temp), &inf);
                }
                if(temp->type == OR && !WIFSIGNALED(inf) && WIFEXITED(inf) && (WEXITSTATUS(inf) != 0)){
                    temp = temp->next;
                    res = exec_conv(temp, len_conv(temp), &inf);
                }
                break;
            }
            exit(res);
        }

        add_elem(bckgrnd, ch_pid);
        return 0;
    }
}

int exec_com_list(tree tr, int len){
    if(tr == NULL) return -1;

    int res;
    tree temp = tr;

    for(int i = 0; i < len; ++i){
        res = exec_com_seq(temp);
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
        if(temp->type == NXT && temp->next != NULL)
            ++len;
    }

    res = exec_com_list(tr, len);
    return res;
}

void fullclearpid(intlist * l){
    intlist prev, temp = *l;
    if(temp != NULL){
        while(temp != NULL){
            prev = temp;
            temp = temp->next;
            free(prev);
        }
        *l = NULL;
    }
}