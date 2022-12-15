CC = cc
LEX = flex
CFLAGS = -Wall -Werror -Wextra
LDFLAGS = -ll
LFlAGS = --noyywrap

all: sh

sh: sh.o tokenize.o
	$(CC) $(CFLAGS) -o $@ ${.ALLSRC} 

sh.o: sh.c
	$(CC) -c sh.c

tokenize.o: tokenize.c
	$(CC) -o $@ -c $> $(LFLAGS) 

tokenize.c: tokenize.l
	$(LEX) $(LFlAGS) -o $@ $>

clean:
	rm -rf *.o tokenize.c tokenize sh
