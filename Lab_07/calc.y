%{
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void yyerror(const char *s);
int yylex(void);
%}

%union {
    double dval;
}

%token <dval> NUMBER
%token LOG EXP_FUNC

%type <dval> expr term factor power

/* Operator precedence (lowest to highest) */
%left '+' '-'
%left '*' '/'
%right '^'
%right UMINUS

%%

program:
    /* empty */
    | program line
    ;

line:
    expr '\n'           { printf("Result = %g\n", $1); }
    | '\n'              { /* blank line */ }
    | error '\n'        { printf("Error: invalid expression.\n"); yyerrok; }
    ;

expr:
    expr '+' term       { $$ = $1 + $3; }
    | expr '-' term     { $$ = $1 - $3; }
    | term              { $$ = $1; }
    ;

term:
    term '*' power      { $$ = $1 * $3; }
    | term '/' power    {
                          if ($3 == 0.0) {
                              yyerror("Division by zero");
                              $$ = 0.0;
                          } else {
                              $$ = $1 / $3;
                          }
                        }
    | power             { $$ = $1; }
    ;

power:
    factor '^' power    { $$ = pow($1, $3); }   /* right-associative */
    | factor            { $$ = $1; }
    ;

factor:
    NUMBER                      { $$ = $1; }
    | '(' expr ')'              { $$ = $2; }
    | '-' factor %prec UMINUS   { $$ = -$2; }
    | LOG '(' expr ')'          {
                                  if ($3 <= 0.0) {
                                      yyerror("log() argument must be positive");
                                      $$ = 0.0;
                                  } else {
                                      $$ = log($3);   /* natural log */
                                  }
                                }
    | EXP_FUNC '(' expr ')'    { $$ = exp($3); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main(void) {
    printf("Arithmetic Expression Evaluator\n");
    printf("Supports: +  -  *  /  ^  log(x)  exp(x)\n");
    printf("Enter expressions (Ctrl+D to quit):\n\n");
    yyparse();
    return 0;
}
