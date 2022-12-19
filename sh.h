#include <sys/wait.h>
#include <sys/stat.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>

#include "list.h"

#define MAX_ARGC 10
#define FG 0
#define BG 1
#define STDIN   0
#define STDOUT  1

FILE *yyin;
int yyparse();

int n_cmd, bg;
Command *tail, *head;
