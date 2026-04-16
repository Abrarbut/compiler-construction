/*
 * sloppy.c  — UNOPTIMIZED (Sloppy) Program
 * CS-346 Compiler Construction | Lab 10: Code Optimization
 * BSCS-2023-CD | NUST SEECS
 *
 * This program intentionally contains common inefficiencies that a
 * compiler optimizer would detect and fix.  See optimized.c for the
 * cleaned-up version that produces IDENTICAL output.
 */

#include <stdio.h>

int main(void)
{
    /* -------------------------------------------------------
     * SLOPPY 1 – CONSTANT FOLDING opportunity
     * All three right-hand sides are constant expressions that
     * could be pre-evaluated at compile time.
     * ------------------------------------------------------- */
    int buffer_size = 8 * 1024;        /* should fold to 8192  */
    int max_tokens  = 50 * 2;          /* should fold to 100   */
    int hash_prime  = 7 * 11 * 13;     /* should fold to 1001  */

    /* -------------------------------------------------------
     * SLOPPY 2 – COPY PROPAGATION opportunity
     * line_count is just a copy of token_count; the variable
     * can be removed and token_count used everywhere.
     * ------------------------------------------------------- */
    int token_count = 100;
    int line_count  = token_count;     /* copy — same value     */

    /* -------------------------------------------------------
     * SLOPPY 3 – CONSTANT PROPAGATION opportunity
     * token_count is a known constant (100); its value can be
     * substituted into every use.
     * ------------------------------------------------------- */
    int symbols     = token_count / 10;               /* 10  */
    int keywords    = token_count / 25;               /* 4   */
    int identifiers = token_count - symbols - keywords; /* 86 */

    /* -------------------------------------------------------
     * SLOPPY 4 – COMMON SUBEXPRESSION ELIMINATION opportunity
     * (token_count + line_count) is computed four times.
     * ------------------------------------------------------- */
    int total_a = (token_count + line_count) * 2;   /* 400 */
    int total_b = (token_count + line_count) * 2;   /* 400 — same as total_a! */
    int total_c = (token_count + line_count) / 2;   /* 100 */
    int total_d = (token_count + line_count) / 2;   /* 100 — same as total_c! */

    /* -------------------------------------------------------
     * SLOPPY 5 – STRENGTH REDUCTION opportunity
     * Multiplications by powers of two can become bit-shifts.
     * ------------------------------------------------------- */
    int mem_a = token_count * 4;    /* should be token_count << 2 */
    int mem_b = token_count * 8;    /* should be token_count << 3 */
    int mem_c = token_count * 16;   /* should be token_count << 4 */
    int mem_d = token_count * 32;   /* should be token_count << 5 */

    /* -------------------------------------------------------
     * SLOPPY 6 – DEAD CODE opportunity
     * These variables are computed but never appear in output.
     * ------------------------------------------------------- */
    int dead_var1 = 9999;
    dead_var1 = dead_var1 * 2 + 1;  /* result never used */
    int dead_var2 = buffer_size * 0; /* always 0, never used */

    /* -------------------------------------------------------
     * SLOPPY 7 – ALGEBRAIC SIMPLIFICATION opportunity
     * x*1==x, x+0==x, x-0==x, x*0==0, x/1==x
     * ------------------------------------------------------- */
    int alg_a = token_count * 1;  /* == token_count */
    int alg_b = token_count + 0;  /* == token_count */
    int alg_c = token_count - 0;  /* == token_count */
    int alg_d = token_count * 0;  /* == 0            */
    int alg_e = token_count / 1;  /* == token_count */

    /* -------------------------------------------------------
     * SLOPPY 8 – LOOP INVARIANT CODE MOTION opportunity
     * (max_tokens * 2) is loop-invariant; it should be hoisted
     * out of the loop body.
     * ------------------------------------------------------- */
    int parse_sum = 0;
    for (int i = 0; i < 10; i++) {
        int loop_const = max_tokens * 2; /* invariant — computed 10 times! */
        parse_sum += i + loop_const;
    }

    /* -------------------------------------------------------
     * SLOPPY 9 – COPY PROPAGATION + UNREACHABLE CODE
     * cp_x and cp_y are redundant copies of token_count.
     * The code after the always-true branch is UNREACHABLE.
     * ------------------------------------------------------- */
    int cp_x = token_count;        /* cp_x = 100       */
    int cp_y = cp_x;               /* cp_y = cp_x = 100 */
    int cp_z = cp_y + 50;          /* 150              */

    int flag = 1;
    if (flag == 1) {
        goto end_processing;       /* always taken → code below is unreachable */
    }
    /* ======= UNREACHABLE BLOCK START ======= */
    printf("This will NEVER print!\n");
    symbols  = symbols  * 999;     /* unreachable dead assignment */
    keywords = keywords + 500;     /* unreachable dead assignment */
    /* ======= UNREACHABLE BLOCK END ========= */

    end_processing: ;

    /* ----- Output ----- */
    printf("===== Compiler Analysis Report =====\n");
    printf("Token Count    : %d\n",  token_count);
    printf("Line Count     : %d\n",  line_count);
    printf("Buffer Size    : %d\n",  buffer_size);
    printf("Max Tokens     : %d\n",  max_tokens);
    printf("Hash Prime     : %d\n",  hash_prime);
    printf("Symbols        : %d\n",  symbols);
    printf("Keywords       : %d\n",  keywords);
    printf("Identifiers    : %d\n",  identifiers);
    printf("Total A        : %d\n",  total_a);
    printf("Total B        : %d\n",  total_b);
    printf("Total C        : %d\n",  total_c);
    printf("Total D        : %d\n",  total_d);
    printf("Mem A (x4)     : %d\n",  mem_a);
    printf("Mem B (x8)     : %d\n",  mem_b);
    printf("Mem C (x16)    : %d\n",  mem_c);
    printf("Mem D (x32)    : %d\n",  mem_d);
    printf("Alg A          : %d\n",  alg_a);
    printf("Alg B          : %d\n",  alg_b);
    printf("Alg C          : %d\n",  alg_c);
    printf("Alg D          : %d\n",  alg_d);
    printf("Alg E          : %d\n",  alg_e);
    printf("Parse Sum      : %d\n",  parse_sum);
    printf("CP Z           : %d\n",  cp_z);
    printf("====================================\n");

    return 0;
}
