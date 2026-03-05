%{
    #include "infix.tab.h"
%}
%option noyywrap

%%

[0-9]+          { yylval = atoi(yytext); return T_Int; }
[-+*/()\n]      { return yytext[0]; }
[ \t]           { /* ignore whitespace */ }
.               { /* ignore */ }

%%