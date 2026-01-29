#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* =======================
   TOKEN DEFINITIONS
   ======================= */

typedef enum {
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_ARITH_OP,
    TOKEN_REL_OP,
    TOKEN_ASSIGN_OP,
    TOKEN_PUNCTUATOR
} TokenType;

/* =======================
   KEYWORD TABLE
   ======================= */

char *keywords[] = {
    "int","float","double","char","if","else","for",
    "while","return","break","continue","void"
};

int isKeyword(char *lexeme) {
    int n = sizeof(keywords) / sizeof(keywords[0]);
    for (int i = 0; i < n; i++) {
        if (strcmp(lexeme, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

/* =======================
   WHITESPACE & COMMENTS
   ======================= */

void skipWhitespace(FILE *fp) {
    char ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (!isspace(ch)) {
            ungetc(ch, fp);
            break;
        }
    }
}

void removeSingleLineComment(FILE *fp) {
    char ch;
    while ((ch = fgetc(fp)) != '\n' && ch != EOF);
}

void removeMultiLineComment(FILE *fp) {
    char ch, prev = 0;
    while ((ch = fgetc(fp)) != EOF) {
        if (prev == '*' && ch == '/')
            break;
        prev = ch;
    }
}

/* =======================
   IDENTIFIER / KEYWORD
   ======================= */

void handleIdentifier(FILE *fp, char firstChar) {
    char buffer[50];
    int i = 0;
    char ch;

    buffer[i++] = firstChar;

    while ((ch = fgetc(fp)) != EOF && (isalnum(ch) || ch == '_')) {
        buffer[i++] = ch;
    }

    buffer[i] = '\0';
    ungetc(ch, fp);

    if (isKeyword(buffer))
        printf("KEYWORD        : %s\n", buffer);
    else
        printf("IDENTIFIER     : %s\n", buffer);
}

/* =======================
   NUMBERS
   ======================= */

void handleNumber(FILE *fp, char firstChar) {
    char buffer[50];
    int i = 0, isFloat = 0;
    char ch;

    buffer[i++] = firstChar;

    while ((ch = fgetc(fp)) != EOF) {
        if (isdigit(ch))
            buffer[i++] = ch;
        else if (ch == '.') {
            isFloat = 1;
            buffer[i++] = ch;
        } else {
            ungetc(ch, fp);
            break;
        }
    }

    buffer[i] = '\0';

    if (isFloat)
        printf("FLOAT          : %s\n", buffer);
    else
        printf("INTEGER        : %s\n", buffer);
}

/* =======================
   SYMBOL TABLE (STUB)
   ======================= */

void addToSymbolTable(char *lexeme) {
    // Stub function (for Part II)
}

/* =======================
   DFA DRIVER
   ======================= */

void lexicalAnalyzer(FILE *fp) {
    char ch;
    int lineNo = 1;

    while ((ch = fgetc(fp)) != EOF) {

        if (ch == '\n')
            lineNo++;

        if (isspace(ch)) {
            skipWhitespace(fp);
        }
        else if (ch == '/') {
            char next = fgetc(fp);
            if (next == '/')
                removeSingleLineComment(fp);
            else if (next == '*')
                removeMultiLineComment(fp);
            else {
                ungetc(next, fp);
                printf("ARITH OP       : /\n");
            }
        }
        else if (isalpha(ch) || ch == '_') {
            handleIdentifier(fp, ch);
        }
        else if (isdigit(ch)) {
            handleNumber(fp, ch);
        }
        else if (ch == '=') {
            char next = fgetc(fp);
            if (next == '=')
                printf("REL OP         : ==\n");
            else {
                ungetc(next, fp);
                printf("ASSIGN OP      : =\n");
            }
        }
        else if (ch == '+' || ch == '-' || ch == '*' || ch == '%') {
            printf("ARITH OP       : %c\n", ch);
        }
        else if (ch == '<' || ch == '>') {
            char next = fgetc(fp);
            if (next == '=')
                printf("REL OP         : %c=\n", ch);
            else {
                ungetc(next, fp);
                printf("REL OP         : %c\n", ch);
            }
        }
        else {
            printf("PUNCTUATOR     : %c\n", ch);
        }
    }
}

/* =======================
   MAIN FUNCTION
   ======================= */

int main() {
    FILE *fp = fopen("input_scanner.c", "r");

    if (fp == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    lexicalAnalyzer(fp);
    fclose(fp);
    return 0;
}
