#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ARGC 10
#define FG 0
#define BG 1
#define STDIN   0
#define STDOUT  1

int yyparse();
void yyerror();

int bg;
struct redirect {
    int fd;
    char *file;
    struct redirect *next;
};
typedef struct redirect Redirect;

struct command {
    int argc;
    char* args[MAX_ARGC];
    
    /* next and prev ommand in the pipe */
    struct command *next;
    struct command *prev;

    /* supports multiple files dupped to strdin/out */
    Redirect *redirects;
};
typedef struct command Command;

Command *tail, *head;

Command*  link_cmd(Command *pipe, Command *cmd);
Command*  cmd_from_red(Redirect *r);
Command*  cmd_from_arg(char *arg);
Command*  add_arg(Command *cmd, char *arg);
Command*  add_red(Command *cmd, Redirect *r);
Redirect* red_out(char *file);
Redirect* red_in(char *file);
Redirect* red_cat(char *file);
