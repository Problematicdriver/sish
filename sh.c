#include "sh.h"
#include <stdio.h>

int yylex();
int chars;
int words;
int lines;

static int f_trace;

int
main() 
{
   (void) yylex();
   printf("%d %d %d\n", chars, words, lines);
   return 0;
}
