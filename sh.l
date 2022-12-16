/* just like Unix wc */
%option  noyywrap
%{
#include "y.tab.h"
#include "sh.h"
extern char *yylval;
%}
BLANK           (" "|\f|\r|\t|\v)
%%

{BLANK}+        { }
[a-zA-Z/-]+       { yylval = strdup(yytext); return WORD; }
">>"            { return GG; }
"|"             { return (int)('|'); }
"<"             { return (int)('<'); }
">"             { return (int)('>'); }
[\n]            { return 0; }

%%
