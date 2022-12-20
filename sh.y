%{
#include "list.h"
%}
%union {
    char*               str;
    struct command*     cmd;
    struct redirect*    redirect;
}

%token GG
%token <str> WORD

%type <cmd>     commandline
%type <cmd>     pipeline
%type <cmd>     command
%type <redirect>redirection

%%
commandline: {;} 
        | pipeline
            /* foreground */
            /* return itself */
            { tail = $1; $$ = $1; }
        | pipeline '&'
            /* background */
            /* set bg and return itself */
            { bg = BG; tail = $1; $$ = $1; }

pipeline: command
            /* return itself */
            { head = $1; n_cmd = 1; $$ = $1; }
        | pipeline '|' command
            /* append command to pipeline, return command */
            { n_cmd++; $$ = link_cmd($1, $3); }

command: WORD
            /* return a command with name field set */
            { $$ = cmd_from_arg($1); }
        | redirection
            /* a command that writes to or read from */
            { $$ = cmd_from_red($1); }
        | command WORD
            /* append word to args of command */
            { $$ = add_arg($1, $2); }
        | command redirection
            /* append file to args of command */
            { $$ = add_red($1, $2); }

redirection: '<' WORD
            /* returns a struct of filename and redirection */
            { $$ = red_in($2); }
        | '>' WORD
            { $$ = red_out($2); }
        | GG WORD
            { $$ = red_cat($2); }
        
%%
void 
yyerror(char *msg)	/* yacc error function */
{
    printf("%s\n" , msg);
}
