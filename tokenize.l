/* just like Unix wc */
%option  noyywrap
%{
#include "sh.h"
int chars;
int words;
int lines;
%}

%%

[a-zA-Z]+  { words++; chars += strlen(yytext); }
\n         { chars++; lines++; }
.          { chars++; }

%%
