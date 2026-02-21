%option noyywrap
%%
(("if"|("then")|("begin")|("end")|("procedure")|("function"))) {printf("A keyword: "); ECHO; printf("\n\n");}
([+-/*]) {printf("An Operator: "); ECHO; printf("\n\n");}
([\n\t" "]) {}
(([0-9]+)(.[0-9]+)?) {printf("A number: "); ECHO; printf("\n\n");}
([a-z]+[0-9a-zA-Z]*) {printf("An id: "); ECHO; printf("\n\n");}

. {printf("Unrecognized character: "); ECHO; printf("\n\n");}
%%

int main(int argc, char **argv)
{

if(argc>1)
yyin=fopen(argv[1],"r");
else
yyin=stdin;
yylex();

}