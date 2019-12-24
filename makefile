PROG = shell
CFLAGS = -g -Wall -fsanitize=address
CC = gcc

$(PROG): main.c list.o exec.o formList.o tree_.o
	$(CC) $(CFLAGS) -c list.c -o list.o
	$(CC) $(CFLAGS) -c tree_.c -o tree_.o
	$(CC) $(CFLAGS) -c exec.c -o exec.o
	$(CC) $(CFLAGS) -c formList.c -o formList.o
	$(CC) $(CFLAGS) main.c list.o exec.o formList.o tree_.o -o $(PROG)
clean:
	rm -f *.o $(PROG)
run:
	rlwrap ./$(PROG)
