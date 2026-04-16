/*
 * optimized.c  — OPTIMIZED Program
 * CS-346 Compiler Construction | Lab 10: Code Optimization
 * BSCS-2023-CD | NUST SEECS
 *
 * Eight optimization techniques applied to sloppy.c.
 * Output is IDENTICAL to sloppy.c.
 */

#include <stdio.h>

int main(void)
{
    /* -------------------------------------------------------
     * OPTIMIZATION 1 – CONSTANT FOLDING
     * Constant-expression RHS values pre-evaluated.
     *   Before: int buffer_size = 8 * 1024;
     *   After : int buffer_size = 8192;
     * ------------------------------------------------------- */
    int buffer_size = 8192;    /* was: 8 * 1024  */
    int max_tokens  = 100;     /* was: 50 * 2    */
    int hash_prime  = 1001;    /* was: 7*11*13   */

    /* -------------------------------------------------------
     * OPTIMIZATION 2 – CONSTANT PROPAGATION
     * token_count = 100 (known constant); value substituted
     * into every use, enabling further simplifications.
     * line_count is a copy of token_count (see OPT 8).
     * ------------------------------------------------------- */
    int token_count = 100;
    /* line_count removed — token_count used directly (OPT 8) */

    int symbols     = 10;   /* was: token_count / 10  */
    int keywords    =  4;   /* was: token_count / 25  */
    int identifiers = 86;   /* was: token_count - symbols - keywords */

    /* -------------------------------------------------------
     * OPTIMIZATION 3 – COMMON SUBEXPRESSION ELIMINATION (CSE)
     * (token_count + token_count) computed ONCE and reused.
     *   Before: four separate (tc + lc) evaluations
     *   After : one shared variable sum_tc
     * ------------------------------------------------------- */
    int sum_tc  = token_count + token_count;  /* shared subexpr = 200 */
    int total_a = sum_tc * 2;   /* was: (tc+lc)*2 — reuses sum_tc  */
    int total_b = total_a;      /* was: (tc+lc)*2 — reuses total_a */
    int total_c = sum_tc / 2;   /* was: (tc+lc)/2 — reuses sum_tc  */
    int total_d = total_c;      /* was: (tc+lc)/2 — reuses total_c */

    /* -------------------------------------------------------
     * OPTIMIZATION 4 – STRENGTH REDUCTION
     * Expensive multiplications replaced by cheap bit-shifts.
     *   Before: token_count * 4
     *   After : token_count << 2
     * ------------------------------------------------------- */
    int mem_a = token_count << 2;   /* was: token_count * 4  */
    int mem_b = token_count << 3;   /* was: token_count * 8  */
    int mem_c = token_count << 4;   /* was: token_count * 16 */
    int mem_d = token_count << 5;   /* was: token_count * 32 */

    /* -------------------------------------------------------
     * OPTIMIZATION 5 – DEAD CODE ELIMINATION
     * dead_var1 and dead_var2 were computed but never used in
     * any output statement → removed entirely.
     * ------------------------------------------------------- */

    /* -------------------------------------------------------
     * OPTIMIZATION 6 – ALGEBRAIC SIMPLIFICATION
     * Applied identity / zero-element rules:
     *   x * 1 = x,  x + 0 = x,  x - 0 = x,
     *   x * 0 = 0,  x / 1 = x
     * ------------------------------------------------------- */
    int alg_a = token_count;   /* was: token_count * 1  (x*1 = x) */
    int alg_b = token_count;   /* was: token_count + 0  (x+0 = x) */
    int alg_c = token_count;   /* was: token_count - 0  (x-0 = x) */
    int alg_d = 0;             /* was: token_count * 0  (x*0 = 0) */
    int alg_e = token_count;   /* was: token_count / 1  (x/1 = x) */

    /* -------------------------------------------------------
     * OPTIMIZATION 7 – LOOP INVARIANT CODE MOTION
     * (max_tokens * 2) does not change across iterations.
     * Hoisted before the loop; also constant-folded to 200
     * because max_tokens was already folded to 100.
     *   Before: inside loop  → int loop_const = max_tokens * 2;
     *   After : outside loop → const 200
     * ------------------------------------------------------- */
    int loop_const = 200;   /* was inside loop: max_tokens * 2 */
    int parse_sum  = 0;
    for (int i = 0; i < 10; i++) {
        parse_sum += i + loop_const;   /* loop_const is now truly invariant */
    }

    /* -------------------------------------------------------
     * OPTIMIZATION 8 – COPY PROPAGATION
     * cp_x = token_count, cp_y = cp_x
     * Both intermediate copies eliminated; token_count used
     * directly.  Also applies to line_count (removed above).
     *   Before: cp_x = tc; cp_y = cp_x; cp_z = cp_y + 50;
     *   After : cp_z = token_count + 50;
     * Unreachable block after always-true branch also removed.
     * ------------------------------------------------------- */
    int cp_z = token_count + 50;   /* was: cp_y + 50 (cp_x, cp_y removed) */

    /* UNREACHABLE CODE ELIMINATED:
     * The if(flag==1) / goto + unreachable printf and dead
     * assignments have been removed entirely. */

    /* ----- Output — identical to sloppy.c ----- */
    printf("===== Compiler Analysis Report =====\n");
    printf("Token Count    : %d\n",  token_count);
    printf("Line Count     : %d\n",  token_count);  /* copy propagation */
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
