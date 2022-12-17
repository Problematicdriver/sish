#include "sh.h"

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
    r->fd = STDOUT;
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
    r->fd = STDIN;
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
    return r;
}

void
print_cmd(Command *c)
{
    char buff[20], in[20], out[20];
    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    int i;
    Redirect *r;
    printf("[%s] ", c->args[0]);
    for (i = 1; i < MAX_ARGC && c->args[i] != NULL; i++) {
        printf("%s ", c->args[i]);
    }
    for (r = c->redirects; r != NULL; r=r->next) {
        if (r->fd == STDIN) {
            strlcat(in, r->file, 20);
            strlcat(in, " ", 20);
        } else {
            strlcat(out, r->file, 20);
            strlcat(in, " ", 20);
        }
    }
    printf("{>%s<%s}", in, out); 
}

void
print_list()
{
    Command* c;
    for (c = head; c != NULL; c = c->next) {
        print_cmd(c);
        printf("%s", c->next == NULL ? "\n" : " -> ");   
    }
}

int
main()
{
    (void)yyparse();
    print_list();
    return 0;
}
