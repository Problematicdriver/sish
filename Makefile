LEX = lex
YACC = yacc
CC = cc

sh: y.tab.o lex.yy.o
	$(CC) -g -o sh sh.c list.c $?

# These dependency rules indicate that (1) lex.yy.o depends on
# lex.yy.c and y.tab.h and (2) lex.yy.o and y.tab.o depend on calc.h.
# Make uses the dependencies to figure out what rules must be run when
# a file has changed.

lex.yy.o: lex.yy.c y.tab.h
lex.yy.o y.tab.o: sh.h list.h

y.tab.c y.tab.h: sh.y
	$(YACC) -d $?

## this is the make rule to use lex to generate the file lex.yy.c from
## our file calc.l

lex.yy.c: sh.l
	$(LEX) $?

## Make clean will delete all of the generated files so we can start
## from scratch

clean:
	-rm -f *.o lex.yy.c *.tab.* sh *.output

