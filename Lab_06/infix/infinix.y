%{
    #include <stdio.h>
    int yylex();
    void yyerror(const char *s);
%}

%token T_Int

%left '+' '-'
%left '*' '/'

%%

S : S E '\n'        { printf("= %d\n", $2); }
  | S '\n'
  |
  ;

E : E '+' E         { $$ = $1 + $3; }
  | E '-' E         { $$ = $1 - $3; }
  | E '*' E         { $$ = $1 * $3; }
  | E '/' E         { $$ = $1 / $3; }
  | '(' E ')'       { $$ = $2; }
  | T_Int            { $$ = $1; }
  ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}
int main() {
    printf("Infix Calculator (e.g., 4 + 8)\n");
    return yyparse();
}