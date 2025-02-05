CC = gcc
CFLAGS = -Wall -g

PROGS   = philosophers_mtx philosophers_sem

all : $(PROGS)
$(PROGS) : % : %.o
	gcc -g -o $@ $^
	
%.o : %.c 
	gcc -c $(CFLAGS) $< -o $@

clean :
	-rm -f *.o $(PROGS) 
