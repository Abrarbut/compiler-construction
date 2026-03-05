%{
    #include <stdio.h>
    int yylex();
    void yyerror(const char *s);
%}

%token T_Int

%%

S : S E '\n'    { printf("= %d\n", $2); }
  |
  ;

E : '+' E E     { $$ = $2 + $3; }
  | '-' E E     { $$ = $2 - $3; }
  | '*' E E     { $$ = $2 * $3; }
  | '/' E E     { $$ = $2 / $3; }
  | T_Int        { $$ = $1; }
  ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}
int main() {
    printf("Prefix Calculator (e.g., + 4 8)\n");
    return yyparse();
}