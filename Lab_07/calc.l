%{
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "calc.tab.h"

void yyerror(const char *s);
%}

%%

[ \t\r]+            { /* discard spaces/tabs/CR */ }
"\n"                { return '\n'; }

[0-9]+\.[0-9]+      { yylval.dval = strtod(yytext, NULL); return NUMBER; }
[0-9]+              { yylval.dval = atof(yytext); return NUMBER; }

"log"               { return LOG; }
"exp"               { return EXP_FUNC; }

"+"                 { return '+'; }
"-"                 { return '-'; }
"*"                 { return '*'; }
"/"                 { return '/'; }
"^"                 { return '^'; }
"("                 { return '('; }
")"                 { return ')'; }

.                   { yyerror("Unknown character"); }

%%

int yywrap(void) {
    return 1;
}
