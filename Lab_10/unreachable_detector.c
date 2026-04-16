/*
 * unreachable_detector.c
 * CS-346 Compiler Construction | Lab 10: Code Optimization
 * BSCS-2023-CD | NUST SEECS
 *
 * Task 2: Automatic detection and removal of unreachable code
 *         using Control Flow Graph (CFG) analysis.
 *
 * Algorithm:
 *   1. Build a CFG whose nodes are the basic blocks of sloppy.c.
 *   2. Perform a Depth-First Search (DFS) from the entry block (B0).
 *   3. Any block not visited is unreachable.
 *   4. Report and "remove" those blocks from the optimized CFG.
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* ============================================================
   Data structures
   ============================================================ */
#define MAX_BLOCKS     16
#define MAX_SUCC        3
#define MAX_LABEL      20
#define MAX_CODE       60

typedef struct {
    int  id;
    char label[MAX_LABEL];
    char code[MAX_CODE];
    int  succ[MAX_SUCC];
    int  num_succ;
    bool reachable;
} BasicBlock;

static BasicBlock cfg[MAX_BLOCKS];
static int        num_blocks = 0;

/* ============================================================
   CFG Construction helpers
   ============================================================ */
static void add_block(int id, const char *label, const char *code)
{
    cfg[id].id        = id;
    strncpy(cfg[id].label, label, MAX_LABEL - 1);
    strncpy(cfg[id].code,  code,  MAX_CODE  - 1);
    cfg[id].num_succ  = 0;
    cfg[id].reachable = false;
    if (id >= num_blocks) num_blocks = id + 1;
}

static void add_edge(int from, int to)
{
    if (cfg[from].num_succ < MAX_SUCC)
        cfg[from].succ[cfg[from].num_succ++] = to;
}

/* ============================================================
   Build the CFG that models sloppy.c
   ============================================================ */
static void build_cfg(void)
{
    /*
     * Block layout (one basic block per group of statements):
     *
     *  B0  ENTRY       – variable declarations (token_count, line_count)
     *  B1  CONST_FOLD  – buffer_size, max_tokens, hash_prime  (OPT-1)
     *  B2  CONST_PROP  – symbols, keywords, identifiers       (OPT-2,3)
     *  B3  CSE         – total_a/b/c/d                         (OPT-4)
     *  B4  STRENGTH    – mem_a/b/c/d                           (OPT-5)
     *  B5  DEAD_CODE   – dead_var1, dead_var2                  (OPT-6)
     *  B6  ALG_SIMP    – alg_a through alg_e                   (OPT-7)
     *  B7  LOOP_BODY   – for-loop (parse_sum)                  (OPT-8)
     *  B8  COPY_PROP   – cp_x, cp_y, cp_z                      (OPT-9)
     *  B9  BRANCH      – if (flag == 1) goto end_processing
     *  B10 UNREACH_1   – printf("NEVER prints!")    ← UNREACHABLE
     *  B11 UNREACH_2   – symbols*=999; keywords+=500← UNREACHABLE
     *  B12 END_PROC    – end_processing: label
     *  B13 OUTPUT      – all printf statements
     *  B14 EXIT        – return 0
     */
    add_block( 0, "ENTRY",      "int token_count=100, lc=tc");
    add_block( 1, "CONST_FOLD", "buffer=8*1024, max=50*2 ...");
    add_block( 2, "CONST_PROP", "symbols=tc/10, kw=tc/25");
    add_block( 3, "CSE",        "total_a=(tc+lc)*2 x4");
    add_block( 4, "STRENGTH",   "mem_a=tc*4, mem_b=tc*8 ..");
    add_block( 5, "DEAD_CODE",  "dead_var1=9999; dead_var2");
    add_block( 6, "ALG_SIMP",   "alg_a=tc*1 ... alg_e=tc/1");
    add_block( 7, "LOOP_BODY",  "for i=0..9: parse_sum+=..");
    add_block( 8, "COPY_PROP",  "cp_x=tc; cp_y=cp_x; cp_z");
    add_block( 9, "BRANCH",     "if(flag==1) goto end_proc");
    add_block(10, "UNREACH_1",  "printf(NEVER prints!)");
    add_block(11, "UNREACH_2",  "symbols*=999; kw+=500");
    add_block(12, "END_PROC",   "end_processing: label");
    add_block(13, "OUTPUT",     "printf(all results)");
    add_block(14, "EXIT",       "return 0");

    /* Control-flow edges  (from → to) */
    add_edge( 0,  1);
    add_edge( 1,  2);
    add_edge( 2,  3);
    add_edge( 3,  4);
    add_edge( 4,  5);
    add_edge( 5,  6);
    add_edge( 6,  7);
    add_edge( 7,  8);
    add_edge( 8,  9);
    add_edge( 9, 12);   /* flag==1 always TRUE → only true-branch edge */
    /* B10, B11 have NO incoming edges → unreachable           */
    add_edge(10, 11);   /* internal unreachable flow (never visited)   */
    add_edge(11, 12);
    add_edge(12, 13);
    add_edge(13, 14);
    /* B14 (EXIT) has no successors */
}

/* ============================================================
   Depth-First Search reachability analysis
   ============================================================ */
static void dfs(int id)
{
    if (id < 0 || id >= num_blocks) return;
    if (cfg[id].reachable)          return;   /* already visited */

    cfg[id].reachable = true;

    for (int i = 0; i < cfg[id].num_succ; i++)
        dfs(cfg[id].succ[i]);
}

/* ============================================================
   Pretty-print helpers
   ============================================================ */
#define HLINE \
    "+-----+------------+-------------------------------+---------------------+\n"

static void print_cfg_table(const char *title)
{
    printf("\n%s\n", title);
    printf(HLINE);
    printf("| ID  | Label      | Code                          | Successors          |\n");
    printf(HLINE);

    for (int i = 0; i < num_blocks; i++) {
        char succ_str[24] = "EXIT";
        if (cfg[i].num_succ > 0) {
            succ_str[0] = '\0';
            for (int j = 0; j < cfg[i].num_succ; j++) {
                char tmp[8];
                if (j) strcat(succ_str, ", ");
                sprintf(tmp, "B%d", cfg[i].succ[j]);
                strcat(succ_str, tmp);
            }
        }
        printf("| B%-2d | %-10s | %-29s | %-19s |\n",
               cfg[i].id, cfg[i].label, cfg[i].code, succ_str);
    }
    printf(HLINE);
}

/* ============================================================
   Main analysis routine
   ============================================================ */
static void analyse_and_remove(void)
{
    /* --- Step 1: DFS from entry node B0 --- */
    dfs(0);

    /* --- Step 2: Report reachability --- */
    printf("\n+-----+------------+-------------------------------+---------------------+\n");
    printf("| ID  | Label      | Code                          | Status              |\n");
    printf("+-----+------------+-------------------------------+---------------------+\n");

    int unreachable_cnt = 0;
    for (int i = 0; i < num_blocks; i++) {
        const char *status = cfg[i].reachable
                             ? "REACHABLE          "
                             : "*** UNREACHABLE ***";
        printf("| B%-2d | %-10s | %-29s | %s |\n",
               cfg[i].id, cfg[i].label, cfg[i].code, status);
        if (!cfg[i].reachable) unreachable_cnt++;
    }
    printf("+-----+------------+-------------------------------+---------------------+\n");

    /* --- Step 3: Summary --- */
    printf("\n[ANALYSIS]  Total blocks    : %d\n", num_blocks);
    printf("[ANALYSIS]  Reachable       : %d\n", num_blocks - unreachable_cnt);
    printf("[ANALYSIS]  Unreachable     : %d\n", unreachable_cnt);

    if (unreachable_cnt > 0) {
        printf("\n[OPTIMIZER] Blocks eliminated (unreachable):\n");
        for (int i = 0; i < num_blocks; i++) {
            if (!cfg[i].reachable)
                printf("  [-] B%d  (%s)  \"%s\"\n",
                       cfg[i].id, cfg[i].label, cfg[i].code);
        }
    } else {
        printf("\n[OPTIMIZER] No unreachable code detected.\n");
    }

    /* --- Step 4: Show cleaned CFG --- */
    printf("\n[RESULT] Optimized CFG (unreachable blocks removed):\n");
    printf("+-----+-------------------------------------------------------------+\n");
    for (int i = 0; i < num_blocks; i++) {
        if (cfg[i].reachable)
            printf("| B%-2d | %-59s |\n", cfg[i].id, cfg[i].code);
    }
    printf("+-----+-------------------------------------------------------------+\n");
}

/* ============================================================
   Entry point
   ============================================================ */
int main(void)
{
    printf("=================================================================\n");
    printf("  CS-346 Compiler Construction  |  Lab 10: Code Optimization\n");
    printf("  Task 2: Unreachable Code Detection via Control Flow Analysis\n");
    printf("  Source program: sloppy.c\n");
    printf("=================================================================\n");

    /* Build the CFG */
    build_cfg();

    /* Print the original CFG */
    print_cfg_table("=== Original Control Flow Graph (CFG) ===");

    /* Run reachability analysis and remove dead blocks */
    printf("\n=== Reachability Analysis Results ===");
    analyse_and_remove();

    printf("\n[DONE] Control flow analysis complete.\n");
    return 0;
}
