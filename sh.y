%{
#define YYSTYPE char *
#include "sh.h"
%}

%token GG WORD

%%
pipeline: command
            { $$ = $1; }
        | pipeline '|' command
            { $$ = print_pipe($1, $3); }

command: WORD
            { $$ = print_exec($1); }
        | redirection
            { $$ = $1; }
        | command WORD
            { $$ = print_cmd($1, $2); }
        | command redirection
            { $$ = print_cmd($1, $2); }
            
redirection: '<' WORD
            { $$ = print_red("<", $2); }
        | '>' WORD
            { $$ = print_red(">", $2); }
        | GG WORD
            { $$ = print_red(">>", $2); }
        
%%
char*
print_pipe(char* s1, char* s2)
{
    char* ss;
    asprintf(&ss, "%s -> %s", s1, s2);
    puts(ss);
    return ss;
}

char*
print_exec(char* s1) {
    char* ss;
    asprintf(&ss, "[%s]", s1);
    return ss;
}

char*
print_cmd(char* s1, char* s2)
{
    char* ss;
    asprintf(&ss, "%s %s", s1, s2);
    return ss;
}

char*
print_red(char* s1, char* s2)
{
    char* ss;
    asprintf(&ss, "%s/%s", s1, s2);
    return ss;
}

void 
yyerror(char *msg)	/* yacc error function */
{
    printf("%s\n" , msg);
}
