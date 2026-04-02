/*
 * Lab 08: FIRST and FOLLOW Sets + LL(1) Parsing Table
 * Course: CS-346 Compiler Construction
 * Name:   Abrar Butt
 * ID:     460297
 *
 * Grammar:
 *   E  -> T E'
 *   E' -> + T E' | - T E' | epsilon
 *   T  -> F T'
 *   T' -> * F T' | / F T' | epsilon
 *   F  -> ( E ) | id | num
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── Constants ─────────────────────────────────────────────────────────── */
#define MAX_NT       10      /* max non-terminals                            */
#define MAX_PROD     20      /* max productions                              */
#define MAX_SYM      20      /* max symbols per production RHS               */
#define MAX_SET      20      /* max elements in a FIRST/FOLLOW set           */
#define MAX_TERM     15      /* max terminals (columns in parse table)       */
#define MAX_NAME     10      /* max chars in a symbol name                   */
#define EPSILON      "eps"
#define ENDMARKER    "$"

/* ─── Data structures ────────────────────────────────────────────────────── */
typedef struct {
    char name[MAX_NAME];
} Symbol;

typedef struct {
    Symbol lhs;                     /* left-hand side non-terminal           */
    Symbol rhs[MAX_SYM];            /* right-hand side symbols               */
    int    rhs_len;                 /* number of symbols on RHS              */
    int    index;                   /* production number (1-based)           */
} Production;

typedef struct {
    Symbol  elems[MAX_SET];
    int     size;
} Set;

/* ─── Globals ────────────────────────────────────────────────────────────── */
Symbol      nonterminals[MAX_NT];
int         nt_count = 0;

Symbol      terminals[MAX_TERM];
int         term_count = 0;

Production  prods[MAX_PROD];
int         prod_count = 0;

Set         FIRST[MAX_NT];          /* FIRST[i] for nonterminals[i]         */
Set         FOLLOW[MAX_NT];         /* FOLLOW[i] for nonterminals[i]        */

/* parse table: table[nt][term] = production index, -1 = error, -2 = synch */
int         table[MAX_NT][MAX_TERM];

/* ─── Helpers ────────────────────────────────────────────────────────────── */
int is_nonterminal(const char *name) {
    for (int i = 0; i < nt_count; i++)
        if (strcmp(nonterminals[i].name, name) == 0) return i;
    return -1;
}

int is_terminal(const char *name) {
    for (int i = 0; i < term_count; i++)
        if (strcmp(terminals[i].name, name) == 0) return i;
    return -1;
}

int in_set(Set *s, const char *sym) {
    for (int i = 0; i < s->size; i++)
        if (strcmp(s->elems[i].name, sym) == 0) return 1;
    return 0;
}

/* returns 1 if something new was added */
int set_add(Set *s, const char *sym) {
    if (in_set(s, sym)) return 0;
    strcpy(s->elems[s->size++].name, sym);
    return 1;
}

/* union src into dst, skip epsilon unless keep_eps; return change flag */
int set_union(Set *dst, Set *src, int keep_eps) {
    int changed = 0;
    for (int i = 0; i < src->size; i++) {
        if (!keep_eps && strcmp(src->elems[i].name, EPSILON) == 0) continue;
        changed |= set_add(dst, src->elems[i].name);
    }
    return changed;
}

int nt_index(const char *name) {
    int i = is_nonterminal(name);
    if (i < 0) { fprintf(stderr, "Unknown NT: %s\n", name); exit(1); }
    return i;
}

int term_index(const char *name) {
    int i = is_terminal(name);
    if (i < 0) { fprintf(stderr, "Unknown term: %s\n", name); exit(1); }
    return i;
}

/* ─── Grammar definition ─────────────────────────────────────────────────── */
void add_nt(const char *name) {
    strcpy(nonterminals[nt_count++].name, name);
}

void add_term(const char *name) {
    strcpy(terminals[term_count++].name, name);
}

void add_prod(const char *lhs, const char **rhs, int rhs_len) {
    Production *p = &prods[prod_count];
    strcpy(p->lhs.name, lhs);
    for (int i = 0; i < rhs_len; i++)
        strcpy(p->rhs[i].name, rhs[i]);
    p->rhs_len = rhs_len;
    p->index   = prod_count + 1;
    prod_count++;
}

void define_grammar(void) {
    /* Non-terminals */
    add_nt("E");   add_nt("E'");
    add_nt("T");   add_nt("T'");
    add_nt("F");

    /* Terminals (including $ and epsilon for display) */
    add_term("+");  add_term("-");
    add_term("*");  add_term("/");
    add_term("(");  add_term(")");
    add_term("id"); add_term("num");
    add_term("$");

    /* Productions */
    /* 1: E  -> T E'       */ { const char *r[]={"T","E'"}; add_prod("E",  r, 2); }
    /* 2: E' -> + T E'     */ { const char *r[]={"+","T","E'"}; add_prod("E'",r,3); }
    /* 3: E' -> - T E'     */ { const char *r[]={"-","T","E'"}; add_prod("E'",r,3); }
    /* 4: E' -> epsilon    */ { const char *r[]={EPSILON};       add_prod("E'",r,1); }
    /* 5: T  -> F T'       */ { const char *r[]={"F","T'"};     add_prod("T", r,2); }
    /* 6: T' -> * F T'     */ { const char *r[]={"*","F","T'"}; add_prod("T'",r,3); }
    /* 7: T' -> / F T'     */ { const char *r[]={"/","F","T'"}; add_prod("T'",r,3); }
    /* 8: T' -> epsilon    */ { const char *r[]={EPSILON};       add_prod("T'",r,1); }
    /* 9: F  -> ( E )      */ { const char *r[]={"(","E",")"}; add_prod("F", r,3); }
    /* 10: F -> id         */ { const char *r[]={"id"};         add_prod("F", r,1); }
    /* 11: F -> num        */ { const char *r[]={"num"};        add_prod("F", r,1); }
}

/* ─── FIRST set computation ──────────────────────────────────────────────── */

/* Compute FIRST of a single symbol and add into dest.
   Returns 1 if epsilon can be derived. */
int first_of_symbol(const char *sym, Set *dest) {
    int ni = is_nonterminal(sym);
    if (ni >= 0) {
        int has_eps = in_set(&FIRST[ni], EPSILON);
        set_union(dest, &FIRST[ni], 1);
        return has_eps;
    }
    /* terminal or epsilon */
    set_add(dest, sym);
    return strcmp(sym, EPSILON) == 0;
}

/* Compute FIRST of a sequence of symbols (RHS of a production).
   Adds results into dest.  Returns 1 if whole sequence can derive epsilon. */
int first_of_sequence(Symbol *seq, int len, Set *dest) {
    for (int i = 0; i < len; i++) {
        int deriv_eps = first_of_symbol(seq[i].name, dest);
        /* remove epsilon from dest unless it's the last symbol */
        if (!deriv_eps) {
            /* remove epsilon we may have added */
            /* (we always add with keep_eps=1 above; clean up) */
            /* easier: rebuild without eps */
            Set tmp; tmp.size = 0;
            for (int k = 0; k < dest->size; k++)
                if (strcmp(dest->elems[k].name, EPSILON) != 0)
                    set_add(&tmp, dest->elems[k].name);
            *dest = tmp;
            return 0;
        }
        /* symbol can derive eps; remove eps tentatively and continue */
        Set tmp; tmp.size = 0;
        for (int k = 0; k < dest->size; k++)
            if (strcmp(dest->elems[k].name, EPSILON) != 0)
                set_add(&tmp, dest->elems[k].name);
        *dest = tmp;
    }
    /* all symbols derived epsilon */
    set_add(dest, EPSILON);
    return 1;
}

void compute_first(void) {
    /* initialise */
    for (int i = 0; i < nt_count; i++) FIRST[i].size = 0;

    int changed = 1;
    while (changed) {
        changed = 0;
        for (int p = 0; p < prod_count; p++) {
            Production *pr = &prods[p];
            int ni = nt_index(pr->lhs.name);
            Set tmp; tmp.size = 0;
            first_of_sequence(pr->rhs, pr->rhs_len, &tmp);
            changed |= set_union(&FIRST[ni], &tmp, 1);
        }
    }
}

/* ─── FOLLOW set computation ─────────────────────────────────────────────── */
void compute_follow(void) {
    for (int i = 0; i < nt_count; i++) FOLLOW[i].size = 0;

    /* $ in FOLLOW of start symbol */
    set_add(&FOLLOW[0], ENDMARKER);

    int changed = 1;
    while (changed) {
        changed = 0;
        for (int p = 0; p < prod_count; p++) {
            Production *pr = &prods[p];
            int lhs_ni = nt_index(pr->lhs.name);
            for (int j = 0; j < pr->rhs_len; j++) {
                int ni = is_nonterminal(pr->rhs[j].name);
                if (ni < 0) continue;  /* skip terminals */

                /* FIRST of the rest of the production */
                Set tmp; tmp.size = 0;
                int rest_eps;
                if (j + 1 < pr->rhs_len) {
                    rest_eps = first_of_sequence(pr->rhs + j + 1,
                                                 pr->rhs_len - j - 1, &tmp);
                } else {
                    rest_eps = 1;
                }
                /* Add FIRST(rest) \ {eps} to FOLLOW[ni] */
                changed |= set_union(&FOLLOW[ni], &tmp, 0);
                /* If rest can derive eps, add FOLLOW(lhs) */
                if (rest_eps)
                    changed |= set_union(&FOLLOW[ni], &FOLLOW[lhs_ni], 0);
            }
        }
    }
}

/* ─── LL(1) table construction ───────────────────────────────────────────── */
void build_table(void) {
    /* initialise with -1 (error) */
    for (int i = 0; i < nt_count; i++)
        for (int j = 0; j < term_count; j++)
            table[i][j] = -1;

    for (int p = 0; p < prod_count; p++) {
        Production *pr = &prods[p];
        int ni = nt_index(pr->lhs.name);

        /* FIRST of production RHS */
        Set tmp; tmp.size = 0;
        int has_eps = first_of_sequence(pr->rhs, pr->rhs_len, &tmp);

        /* For each terminal a in FIRST(rhs) \ {eps}: table[A][a] = p */
        for (int k = 0; k < tmp.size; k++) {
            if (strcmp(tmp.elems[k].name, EPSILON) == 0) continue;
            int ti = is_terminal(tmp.elems[k].name);
            if (ti < 0) continue;
            if (table[ni][ti] == -1)
                table[ni][ti] = pr->index;
            else
                printf("CONFLICT at [%s][%s]\n", pr->lhs.name, tmp.elems[k].name);
        }

        /* If eps in FIRST(rhs): for each terminal b in FOLLOW(A): table[A][b]=p */
        if (has_eps) {
            for (int k = 0; k < FOLLOW[ni].size; k++) {
                int ti = is_terminal(FOLLOW[ni].elems[k].name);
                if (ti < 0) continue;
                if (table[ni][ti] == -1)
                    table[ni][ti] = pr->index;
                else
                    printf("CONFLICT at [%s][%s]\n",
                           pr->lhs.name, FOLLOW[ni].elems[k].name);
            }
        }
    }
}

/* ─── Display helpers ────────────────────────────────────────────────────── */
void print_set(Set *s) {
    printf("{ ");
    for (int i = 0; i < s->size; i++) {
        printf("%s", s->elems[i].name);
        if (i < s->size - 1) printf(", ");
    }
    printf(" }");
}

void print_productions(void) {
    printf("\n=== Grammar Productions ===\n");
    for (int p = 0; p < prod_count; p++) {
        Production *pr = &prods[p];
        printf("  %2d. %s -> ", pr->index, pr->lhs.name);
        for (int j = 0; j < pr->rhs_len; j++) {
            printf("%s", pr->rhs[j].name);
            if (j < pr->rhs_len - 1) printf(" ");
        }
        printf("\n");
    }
}

void print_first_follow(void) {
    printf("\n=== FIRST and FOLLOW Sets ===\n");
    printf("%-10s %-10s %-30s %-30s\n",
           "NT", "Nullable", "FIRST", "FOLLOW");
    printf("%-10s %-10s %-30s %-30s\n",
           "----------","----------",
           "------------------------------",
           "------------------------------");
    for (int i = 0; i < nt_count; i++) {
        printf("%-10s ", nonterminals[i].name);
        int nullable = in_set(&FIRST[i], EPSILON);
        printf("%-10s ", nullable ? "Yes" : "No");
        /* print FIRST without eps for cleanliness, mark with e */
        printf("{ ");
        int first_printed = 0;
        for (int k = 0; k < FIRST[i].size; k++) {
            if (first_printed) printf(", ");
            printf("%s", FIRST[i].elems[k].name);
            first_printed = 1;
        }
        printf(" }");
        /* padding */
        int len = 2;
        for (int k = 0; k < FIRST[i].size; k++) len += strlen(FIRST[i].elems[k].name)+2;
        while (len++ < 32) printf(" ");

        printf("{ ");
        for (int k = 0; k < FOLLOW[i].size; k++) {
            printf("%s", FOLLOW[i].elems[k].name);
            if (k < FOLLOW[i].size - 1) printf(", ");
        }
        printf(" }\n");
    }
}

void print_table(void) {
    printf("\n=== LL(1) Parsing Table ===\n");
    /* header */
    printf("%-8s", "NT");
    for (int j = 0; j < term_count; j++)
        printf(" %-10s", terminals[j].name);
    printf("\n");
    printf("--------");
    for (int j = 0; j < term_count; j++) printf(" ----------");
    printf("\n");

    for (int i = 0; i < nt_count; i++) {
        printf("%-8s", nonterminals[i].name);
        for (int j = 0; j < term_count; j++) {
            int v = table[i][j];
            if (v == -1)       printf(" %-10s", "error");
            else if (v == -2)  printf(" %-10s", "synch");
            else {
                /* print as "NT->rhs" abbreviated */
                Production *pr = &prods[v-1];
                char buf[32];
                snprintf(buf, sizeof(buf), "%d:%s->", v, pr->lhs.name);
                for (int k = 0; k < pr->rhs_len && strlen(buf)<28; k++)
                    strncat(buf, pr->rhs[k].name, sizeof(buf)-strlen(buf)-1);
                printf(" %-10s", buf);
            }
        }
        printf("\n");
    }
}

/* ─── Main ───────────────────────────────────────────────────────────────── */
int main(void) {

    define_grammar();
    print_productions();

    compute_first();
    compute_follow();
    print_first_follow();

    build_table();
    print_table();

    printf("\n[Done]\n");
    return 0;
}
