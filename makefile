PROG = shell
CFLAGS = -g -Wall
CC = gcc

$(PROG): main.c list.o tree.o exec.o formList.o
	$(CC) $(CFLAGS) -c list.c -o list.o
	$(CC) $(CFLAGS) -c print_tree.c -o print_tree.o
	$(CC) $(CFLAGS) -c tree.c -o tree.o
	$(CC) $(CFLAGS) -c exec.c -o exec.o
	$(CC) $(CFLAGS) -c formList.c -o formList.o
	$(CC) $(CFLAGS) main.c list.o tree.o exec.o formList.o -o $(PROG)