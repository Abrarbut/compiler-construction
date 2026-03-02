%{
#include <stdio.h>
#include <stdlib.h>

int stack[100];
int top = -1;

void push(int val) {
    stack[++top] = val;
    printf("Push %d -> Stack: [", val);
    for (int i = 0; i <= top; i++) {
        printf("%d", stack[i]);
        if (i < top) printf(", ");
    }
    printf("]\n");
}

int pop() {
    if (top < 0) {
        printf("Error: stack underflow\n");
        exit(1);
    }
    return stack[top--];
}

void print_stack() {
    printf("Stack: [");
    for (int i = 0; i <= top; i++) {
        printf("%d", stack[i]);
        if (i < top) printf(", ");
    }
    printf("]\n");
}
%}

DIGIT [0-9]
NUM   {DIGIT}+

%%

{NUM}   {
    int val = atoi(yytext);
    push(val);
}

"+"     {
    int b = pop(), a = pop();
    int result = a + b;
    printf("Pop %d, Pop %d -> %d + %d = %d\n", b, a, a, b, result);
    push(result);
}

"-"     {
    int b = pop(), a = pop();
    int result = a - b;
    printf("Pop %d, Pop %d -> %d - %d = %d\n", b, a, a, b, result);
    push(result);
}

"*"     {
    int b = pop(), a = pop();
    int result = a * b;
    printf("Pop %d, Pop %d -> %d * %d = %d\n", b, a, a, b, result);
    push(result);
}

[ \t\n]+ { /* skip whitespace */ }

.       { printf("UNRECOGNIZED: %s\n", yytext); }

%%

int yywrap() { return 1; }

int main() {
    printf("Enter postfix expression (Ctrl+D to end):\n");
    yylex();
    if (top >= 0)
        printf("\nFinal Result: %d\n", stack[top]);
    else
        printf("Empty expression.\n");
    return 0;
}
