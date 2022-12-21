#include <sys/wait.h>
#include <sys/stat.h>

#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include "list.h"

int yyparse();
FILE* yyin;

void free_list();
