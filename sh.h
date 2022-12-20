#include <sys/wait.h>
#include <sys/stat.h>

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>

#include "list.h"

#define MAX_ARGC 10
#define FG 0
#define BG 1

int yyparse();
void yyerror();
FILE* yyin;
