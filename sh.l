/* just like Unix wc */
%option  noyywrap
%{
#include "y.tab.h"
#include "sh.h"
%}
BLANK           (" "|\f|\r|\t|\v)
%%

{BLANK}+        { }
[a-zA-Z0-9/\-.$?]+ { yylval.str = strdup(yytext); return WORD; }
">>"            { return GG; }
"|"             { return (int)('|'); }
"<"             { return (int)('<'); }
">"             { return (int)('>'); }
"&"             { return (int)('&'); }
<<EOF>>         { yyterminate(); return 0; }
%%
