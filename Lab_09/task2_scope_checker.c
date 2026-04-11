/*
 * Lab 09 - Task 2: Scope Checking (Scanner + Parser)
 * Course: CS-346 Compiler Construction
 * NUST - SEECS
 *
 * Compile: gcc task2_scope_checker.c -o task2
 * Run:     ./task2
 *
 * Demonstrates:
 *   - Lexical scanning (tokenizer)
 *   - Recursive-descent parsing
 *   - Scoped symbol table (global + local)
 *   - Local variable shadows global variable
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ==================================================
   SECTION 1 -- LEXER / SCANNER
   ================================================== */

typedef enum {
    /* keywords */
    TOK_INT, TOK_FLOAT, TOK_RETURN,
    /* identifiers & literals */
    TOK_IDENT, TOK_NUMBER,
    /* operators & punctuation */
    TOK_ASSIGN,         /* =  */
    TOK_SEMICOLON,      /* ;  */
    TOK_LBRACE,         /* {  */
    TOK_RBRACE,         /* }  */
    TOK_LPAREN,         /* (  */
    TOK_RPAREN,         /* )  */
    TOK_LSHIFT,         /* << */
    TOK_COMMA,          /* ,  */
    /* I/O */
    TOK_COUT,
    /* special */
    TOK_EOF,
    TOK_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char      lexeme[64];
    int       line;
} Token;

/* -- Lexer State -- */
static const char* src;
static int         srcPos;
static int         lineNo;

void lexerInit(const char* code) {
    src    = code;
    srcPos = 0;
    lineNo = 1;
}

static void skipWhitespaceAndComments(void) {
    for (;;) {
        /* whitespace */
        while (src[srcPos] && isspace(src[srcPos])) {
            if (src[srcPos] == '\n') lineNo++;
            srcPos++;
        }
        /* single-line comment */
        if (src[srcPos] == '/' && src[srcPos+1] == '/') {
            while (src[srcPos] && src[srcPos] != '\n') srcPos++;
            continue;
        }
        break;
    }
}

Token lexerNext(void) {
    Token tok;
    tok.lexeme[0] = '\0';
    tok.line      = lineNo;

    skipWhitespaceAndComments();

    if (!src[srcPos]) {
        tok.type = TOK_EOF;
        strcpy(tok.lexeme, "EOF");
        return tok;
    }

    /* Identifier or keyword */
    if (isalpha(src[srcPos]) || src[srcPos] == '_') {
        int i = 0;
        while (isalnum(src[srcPos]) || src[srcPos] == '_')
            tok.lexeme[i++] = src[srcPos++];
        tok.lexeme[i] = '\0';

        if      (!strcmp(tok.lexeme, "int"))    tok.type = TOK_INT;
        else if (!strcmp(tok.lexeme, "float"))  tok.type = TOK_FLOAT;
        else if (!strcmp(tok.lexeme, "return")) tok.type = TOK_RETURN;
        else if (!strcmp(tok.lexeme, "cout"))   tok.type = TOK_COUT;
        else                                    tok.type = TOK_IDENT;
        return tok;
    }

    /* Number literal (integer or float) */
    if (isdigit(src[srcPos])) {
        int i = 0;
        while (isdigit(src[srcPos]) || src[srcPos] == '.')
            tok.lexeme[i++] = src[srcPos++];
        tok.lexeme[i] = '\0';
        tok.type = TOK_NUMBER;
        return tok;
    }

    /* Two-character tokens */
    if (src[srcPos] == '<' && src[srcPos+1] == '<') {
        strcpy(tok.lexeme, "<<");
        tok.type = TOK_LSHIFT;
        srcPos += 2;
        return tok;
    }

    /* Single-character tokens */
    tok.lexeme[0] = src[srcPos];
    tok.lexeme[1] = '\0';
    switch (src[srcPos++]) {
        case '=': tok.type = TOK_ASSIGN;    break;
        case ';': tok.type = TOK_SEMICOLON; break;
        case '{': tok.type = TOK_LBRACE;    break;
        case '}': tok.type = TOK_RBRACE;    break;
        case '(': tok.type = TOK_LPAREN;    break;
        case ')': tok.type = TOK_RPAREN;    break;
        case ',': tok.type = TOK_COMMA;     break;
        default:  tok.type = TOK_UNKNOWN;   break;
    }
    return tok;
}

/* -- Token Stream (pre-tokenised array) -- */
#define MAX_TOKENS 512
static Token  toks[MAX_TOKENS];
static int    tokCount = 0;
static int    tokCur   = 0;

void tokenize(const char* code) {
    lexerInit(code);
    Token t;
    do {
        t = lexerNext();
        toks[tokCount++] = t;
    } while (t.type != TOK_EOF && tokCount < MAX_TOKENS - 1);
}

void printTokens(void) {
    printf("--- Token Stream ---------------------------\n");
    printf("  %-4s  %-12s  %s\n", "Line", "Type", "Lexeme");
    for (int i = 0; i < tokCount; i++) {
        const char* tn = "UNKNOWN";
        switch (toks[i].type) {
            case TOK_INT:       tn = "INT";       break;
            case TOK_FLOAT:     tn = "FLOAT";     break;
            case TOK_RETURN:    tn = "RETURN";    break;
            case TOK_IDENT:     tn = "IDENT";     break;
            case TOK_NUMBER:    tn = "NUMBER";    break;
            case TOK_ASSIGN:    tn = "ASSIGN";    break;
            case TOK_SEMICOLON: tn = "SEMICOLON"; break;
            case TOK_LBRACE:    tn = "LBRACE";    break;
            case TOK_RBRACE:    tn = "RBRACE";    break;
            case TOK_LPAREN:    tn = "LPAREN";    break;
            case TOK_RPAREN:    tn = "RPAREN";    break;
            case TOK_LSHIFT:    tn = "LSHIFT";    break;
            case TOK_COUT:      tn = "COUT";      break;
            case TOK_EOF:       tn = "EOF";       break;
            default:                               break;
        }
        printf("  %-4d  %-12s  \"%s\"\n", toks[i].line, tn, toks[i].lexeme);
    }
    printf("\n");
}

/* Peek / consume helpers */
static Token peekTok(void)          { return toks[tokCur]; }
static Token consumeTok(void)       { return toks[tokCur++]; }
static Token expectTok(TokenType t) {
    Token tk = consumeTok();
    if (tk.type != t)
        printf("  [Parse Error] Line %d: unexpected token \"%s\"\n",
               tk.line, tk.lexeme);
    return tk;
}


/* ==================================================
   SECTION 2 -- SCOPED SYMBOL TABLE
   ================================================== */

#define MAX_SCOPE_DEPTH 16
#define MAX_SYMS_PER_SCOPE 64

typedef struct {
    char name[64];
    char type[16];
    char value[64];
} SymEntry;

typedef struct {
    SymEntry entries[MAX_SYMS_PER_SCOPE];
    int      count;
    char     label[32];   /* "global" / "main" / ... */
} ScopeLevel;

static ScopeLevel scopeStack[MAX_SCOPE_DEPTH];
static int        scopeDepth = -1;

void scopeEnter(const char* label) {
    scopeDepth++;
    scopeStack[scopeDepth].count = 0;
    strncpy(scopeStack[scopeDepth].label, label, 31);
    printf("  [Scope] >>> Enter scope: \"%s\"\n", label);
}

void scopeExit(void) {
    printf("  [Scope] <<< Exit  scope: \"%s\"\n",
           scopeStack[scopeDepth].label);
    scopeDepth--;
}

/* Declare a symbol in the CURRENT (innermost) scope */
int scopeDeclare(const char* name, const char* type, const char* value) {
    ScopeLevel* cur = &scopeStack[scopeDepth];
    /* check for redeclaration in same scope */
    for (int i = 0; i < cur->count; i++) {
        if (!strcmp(cur->entries[i].name, name)) {
            printf("  [Semantic Error] Redeclaration of \"%s\" in scope \"%s\"\n",
                   name, cur->label);
            return 0;
        }
    }
    SymEntry* e = &cur->entries[cur->count++];
    strncpy(e->name,  name,  63);
    strncpy(e->type,  type,  15);
    strncpy(e->value, value, 63);
    printf("  [Declare] %-6s %-6s = %-6s  (scope: \"%s\")\n",
           type, name, value, cur->label);
    return 1;
}

/* Lookup -- innermost scope wins (shadowing) */
SymEntry* scopeLookup(const char* name) {
    for (int d = scopeDepth; d >= 0; d--) {
        ScopeLevel* sc = &scopeStack[d];
        for (int i = 0; i < sc->count; i++) {
            if (!strcmp(sc->entries[i].name, name))
                return &sc->entries[i];
        }
    }
    return NULL;
}


/* ==================================================
   SECTION 3 -- PARSER
   ================================================== */

static void parseBlock(void);

/* Parse:  <type> <ident> = <value> ; */
static void parseVarDecl(void) {
    Token typeTok  = consumeTok();            /* int / float       */
    Token nameTok  = expectTok(TOK_IDENT);    /* variable name     */
    expectTok(TOK_ASSIGN);                    /* =                 */
    Token valTok   = consumeTok();            /* number literal    */
    expectTok(TOK_SEMICOLON);                 /* ;                 */
    scopeDeclare(nameTok.lexeme, typeTok.lexeme, valTok.lexeme);
}

/* Parse:  cout << <expr> ; */
static void parseCout(void) {
    consumeTok();                  /* cout */
    expectTok(TOK_LSHIFT);         /* <<   */
    Token exprTok = consumeTok();  /* ident or literal */

    if (exprTok.type == TOK_IDENT) {
        SymEntry* e = scopeLookup(exprTok.lexeme);
        if (e) {
            printf("  [cout] \"%s\" resolved to value = %s"
                   "  (from scope \"%s\")\n",
                   exprTok.lexeme, e->value,
                   scopeStack[scopeDepth - 
                       /* find which depth */ 0].label);
            /* reprint clearly */
            /* find the actual scope for display */
            for (int d = scopeDepth; d >= 0; d--) {
                ScopeLevel* sc = &scopeStack[d];
                for (int i = 0; i < sc->count; i++) {
                    if (!strcmp(sc->entries[i].name, exprTok.lexeme)) {
                        printf("         (declared in scope \"%s\")\n",
                               sc->label);
                        goto found;
                    }
                }
            }
            found:;
        } else {
            printf("  [Semantic Error] Undeclared variable \"%s\" at line %d\n",
                   exprTok.lexeme, exprTok.line);
        }
    }
    /* skip rest of cout chain until semicolon */
    while (peekTok().type != TOK_SEMICOLON &&
           peekTok().type != TOK_EOF)
        consumeTok();
    expectTok(TOK_SEMICOLON);
}

/* Parse:  return <expr> ; */
static void parseReturn(void) {
    consumeTok();   /* return */
    while (peekTok().type != TOK_SEMICOLON &&
           peekTok().type != TOK_EOF)
        consumeTok();
    expectTok(TOK_SEMICOLON);
}

/* Parse block body { ... } */
static void parseBlock(void) {
    expectTok(TOK_LBRACE);
    while (peekTok().type != TOK_RBRACE &&
           peekTok().type != TOK_EOF) {
        Token t = peekTok();
        if (t.type == TOK_INT || t.type == TOK_FLOAT)
            parseVarDecl();
        else if (t.type == TOK_COUT)
            parseCout();
        else if (t.type == TOK_RETURN)
            parseReturn();
        else
            consumeTok();   /* skip unsupported constructs */
    }
    expectTok(TOK_RBRACE);
}

/* Parse a function definition:  <type> <name> () { ... } */
static void parseFunction(void) {
    consumeTok();                    /* return type  */
    Token nameTok = consumeTok();    /* function name*/
    expectTok(TOK_LPAREN);
    expectTok(TOK_RPAREN);
    scopeEnter(nameTok.lexeme);
    parseBlock();
    scopeExit();
}

/* Top-level parse */
void parse(void) {
    scopeEnter("global");
    while (peekTok().type != TOK_EOF) {
        Token t = peekTok();
        if (t.type == TOK_INT || t.type == TOK_FLOAT) {
            /* lookahead: is this a function or a global var? */
            if (tokCur + 2 < tokCount &&
                toks[tokCur + 2].type == TOK_LPAREN)
                parseFunction();
            else
                parseVarDecl();
        } else {
            consumeTok();
        }
    }
    scopeExit();
}


/* ==================================================
   SECTION 4 -- MAIN
   ================================================== */
int main(void) {
    printf("+==========================================+\n");
    printf("|  Lab 09 - Task 2: Scope Checker           |\n");
    printf("|  (Scanner + Parser + Scoped Symbol Table) |\n");
    printf("+==========================================+\n\n");

    /* -- Source code under analysis -- */
    const char* code =
        "// Global variable declaration\n"
        "int g = 20;\n"
        "\n"
        "int main() {\n"
        "    // Local variable declaration\n"
        "    int g = 10;\n"
        "    cout << g;\n"
        "    return 0;\n"
        "}\n";

    printf("--- Input Source Code ----------------------\n");
    printf("%s\n", code);

    /* -- Step 1: Scan / Tokenise -- */
    printf("--- Step 1: Scanning (Tokenisation) --------\n");
    tokenize(code);
    printTokens();

    /* -- Step 2: Parse + Scope Check -- */
    printf("--- Step 2: Parsing & Scope Analysis -------\n");
    parse();

    /* -- Step 3: Summary -- */
    printf("\n--- Scope Check Result ---------------------\n");
    printf("  Global  : int g = 20\n");
    printf("  Local   : int g = 10  (inside main)\n");
    printf("  Output  : cout << g  -->  10\n");
    printf("  Reason  : Local variable shadows global variable.\n");
    printf("            Innermost scope takes precedence.\n");
    printf("--- End of Task 2 --------------------------\n");
    return 0;
}
