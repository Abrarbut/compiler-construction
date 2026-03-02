%{
#include <stdio.h>
%}

DIGIT    [0-9]
LETTER   [a-z]
INT      {DIGIT}+
FLOAT    {DIGIT}+"."{DIGIT}+
ID       {LETTER}([a-zA-Z0-9])*

%%

"if"        { printf("KEYWORD: %s\n", yytext); }
"then"      { printf("KEYWORD: %s\n", yytext); }
"begin"     { printf("KEYWORD: %s\n", yytext); }
"end"       { printf("KEYWORD: %s\n", yytext); }
"procedure" { printf("KEYWORD: %s\n", yytext); }
"function"  { printf("KEYWORD: %s\n", yytext); }

{FLOAT}     { printf("FLOAT: %s\n", yytext); }
{INT}       { printf("INTEGER: %s\n", yytext); }
{ID}        { printf("IDENTIFIER: %s\n", yytext); }

"+"         { printf("OPERATOR: +\n"); }
"-"         { printf("OPERATOR: -\n"); }
"*"         { printf("OPERATOR: *\n"); }
"/"         { printf("OPERATOR: /\n"); }

[ \t\n]+    { /* skip whitespace */ }

.           { printf("UNRECOGNIZED: %s\n", yytext); }

%%

int yywrap() { return 1; }

int main() {
    printf("Enter source code (Ctrl+D to end):\n");
    yylex();
    return 0;
}
