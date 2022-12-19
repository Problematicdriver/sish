#include "list.h"

Command*
link_cmd(Command *pipe, Command *cmd)
{
    pipe->next = cmd;
    cmd->prev = pipe;
    return cmd;
}

Command*
cmd_from_arg(char *arg)
{
    Command* cmd;
    if ((cmd = malloc(sizeof(Command))) == NULL) {
        perror("malloc");
    }
    memset(cmd, 0, sizeof(struct command));
    cmd->args[cmd->argc] = arg;
    cmd->argc++;
    return cmd;
}

Command*
cmd_from_red(Redirect *r)
{
    Command *cmd;
    if ((cmd = malloc(sizeof(Command))) == NULL) {
        perror("malloc");
    }
    memset(cmd, 0, sizeof(Command));
    cmd->redirects = r;
    return cmd;
}

Command*  
add_red(Command *cmd, Redirect *r)
{
    r->next = cmd->redirects;
    cmd->redirects = r;
    return cmd;
}

Command*
add_arg(Command *cmd, char *arg)
{
    cmd->args[cmd->argc] = arg;
    cmd->argc++;
    return cmd;
}

Redirect*
red_out(char *file)
{
    Redirect *r;
    if ((r = malloc(sizeof(Redirect))) == NULL) {
        perror("malloc");
    }
    memset(r, 0, sizeof(Redirect));
    r->file = file;
    r->red_fileno = STDOUT_FILENO;
    return r;
}

Redirect*
red_in(char *file)
{
    Redirect *r;
    if ((r = malloc(sizeof(Redirect))) == NULL) {
         perror("malloc");
    }
    memset(r, 0, sizeof(Redirect));
    r->file = file;
    r->red_fileno = STDIN_FILENO;
    return r;
}

Redirect*
red_cat(char *file)
{
    Redirect *r;
    if ((r = malloc(sizeof(Redirect))) == NULL) {
        perror("malloc");
    }
    memset(r, 0, sizeof(Redirect));
    r->file = file;
    r->red_fileno = STDOUT_FILENO;
    r->f_cat = 1;
    return r;
}
