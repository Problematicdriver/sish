#include <sys/wait.h>
#include <sys/stat.h>

#include <assert.h>
#include <fcntl.h>

#include "list.h"

int yyparse();
FILE* yyin;
