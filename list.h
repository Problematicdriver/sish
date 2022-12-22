#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_ARGC 10
#define FG 0
#define BG 1

int bg, n_cmd;
struct redirect {
    int red_fileno;
    int f_cat;
    char *file;
    struct redirect *next;
};
typedef struct redirect Redirect;

struct command {
    int argc;
    char* args[MAX_ARGC];
   
    /* next ommand in the pipe */
    struct command *next;

    /* supports multiple files dupped to strdin/out */
    Redirect *redirects;
};
typedef struct command Command;

Command *head, *tail;

Command*  link_cmd(Command *pipe, Command *cmd);
Command*  cmd_from_red(Redirect *r);
Command*  cmd_from_arg(char *arg);
Command*  add_arg(Command *cmd, char *arg);
Command*  add_red(Command *cmd, Redirect *r);
Redirect* red_out(char *file);
Redirect* red_in(char *file);
Redirect* red_cat(char *file);
