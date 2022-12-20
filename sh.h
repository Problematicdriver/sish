#include <sys/wait.h>
#include <sys/stat.h>

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>

#include "list.h"

int yyparse();
FILE* yyin;
