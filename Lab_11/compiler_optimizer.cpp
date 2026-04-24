/*
 * CS-346 Compiler Construction - Lab 11: Code Optimization
 * NUST SEECS - BSCS-2023-CD
 *
 * This file implements a mini-compiler with:
 *   Task 1: Modular compiler stages
 *   Task 2: Three Address Code (TAC) IR generation
 *   Task 3: Constant Folding, Constant Propagation,
 *           Algebraic Simplification, CSE
 *   Task 4: Unreachable/Dead Code Elimination
 *   Task 5: Loop Invariant Code Motion, Loop Fusion,
 *           Strength Reduction
 *   Task 6: Execution time comparison
 *
 * GitHub: https://github.com/Abrarbut/compiler-construction.git
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <regex>
#include <chrono>
#include <algorithm>
#include <iomanip>

// ============================================================
// =================== DATA STRUCTURES ========================
// ============================================================

struct Token {
    std::string type;
    std::string value;
};

struct TACInstruction {
    std::string result;
    std::string op;
    std::string arg1;
    std::string arg2;
    bool isLabel    = false;
    bool isJump     = false;
    bool isReturn   = false;
    bool isLoopHead = false;   // marks: FOR_BEGIN / WHILE_BEGIN
    bool isLoopEnd  = false;   // marks: FOR_END   / WHILE_END
    std::string raw;           // original text line
};

// ============================================================
// =================== TASK 1: LEXER ==========================
// ============================================================

class Lexer {
public:
    std::vector<Token> tokenize(const std::string& src) {
        std::vector<Token> tokens;
        size_t i = 0;
        while (i < src.size()) {
            // Skip whitespace
            if (std::isspace(src[i])) { ++i; continue; }

            // Single-line comment
            if (i + 1 < src.size() && src[i] == '/' && src[i+1] == '/') {
                while (i < src.size() && src[i] != '\n') ++i;
                continue;
            }

            // Number literal
            if (std::isdigit(src[i])) {
                std::string num;
                while (i < src.size() && std::isdigit(src[i])) num += src[i++];
                tokens.push_back({"NUMBER", num});
                continue;
            }

            // Identifier or keyword
            if (std::isalpha(src[i]) || src[i] == '_') {
                std::string word;
                while (i < src.size() && (std::isalnum(src[i]) || src[i] == '_'))
                    word += src[i++];
                static const std::set<std::string> kw =
                    {"int","float","return","if","else","for","while","void"};
                tokens.push_back({kw.count(word) ? "KEYWORD" : "IDENT", word});
                continue;
            }

            // Two-char operators
            if (i + 1 < src.size()) {
                std::string two = {src[i], src[i+1]};
                if (two == "==" || two == "!=" || two == "<=" || two == ">=") {
                    tokens.push_back({"RELOP", two}); i += 2; continue;
                }
            }

            // Single-char symbols
            std::string ch(1, src[i]);
            if (ch == "+" || ch == "-" || ch == "*" || ch == "/" || ch == "%")
                tokens.push_back({"OP", ch});
            else if (ch == "=" )
                tokens.push_back({"ASSIGN", ch});
            else if (ch == ";" )
                tokens.push_back({"SEMI", ch});
            else if (ch == "(" || ch == ")")
                tokens.push_back({"PAREN", ch});
            else if (ch == "{" || ch == "}")
                tokens.push_back({"BRACE", ch});
            else if (ch == "<" || ch == ">")
                tokens.push_back({"RELOP", ch});
            else if (ch == ",")
                tokens.push_back({"COMMA", ch});
            else
                tokens.push_back({"UNKNOWN", ch});
            ++i;
        }
        return tokens;
    }

    void printTokens(const std::vector<Token>& tokens) {
        std::cout << "\n=== LEXER OUTPUT ===\n";
        for (auto& t : tokens)
            std::cout << "  [" << std::setw(8) << t.type << "] " << t.value << "\n";
    }
};

// ============================================================
// ============= TASK 1 & 2: PARSER + IR GENERATOR ============
// ============================================================

class IRGenerator {
    int tempCount = 0;
    std::string newTemp() { return "t" + std::to_string(++tempCount); }

public:
    std::vector<TACInstruction> instructions;

    // --- helper emit functions ---
    void emit(const std::string& result,
              const std::string& op,
              const std::string& arg1,
              const std::string& arg2 = "") {
        TACInstruction i;
        i.result = result; i.op = op; i.arg1 = arg1; i.arg2 = arg2;
        std::string raw = result + " = " + arg1;
        if (!op.empty())   raw += " " + op + " " + arg2;
        i.raw = raw;
        instructions.push_back(i);
    }

    void emitLabel(const std::string& lbl) {
        TACInstruction i; i.isLabel = true;
        i.result = lbl; i.raw = lbl + ":";
        instructions.push_back(i);
    }

    void emitJump(const std::string& target,
                  const std::string& cond = "",
                  const std::string& cmp  = "") {
        TACInstruction i; i.isJump = true;
        i.result = target; i.arg1 = cond; i.arg2 = cmp;
        if (cond.empty())
            i.raw = "goto " + target;
        else
            i.raw = "if " + cond + " goto " + target;
        instructions.push_back(i);
    }

    void emitReturn(const std::string& val) {
        TACInstruction i; i.isReturn = true; i.arg1 = val;
        i.raw = "return " + val;
        instructions.push_back(i);
    }

    void emitLoopHead(const std::string& label) {
        TACInstruction i; i.isLoopHead = true;
        i.result = label; i.raw = "LOOP_BEGIN " + label;
        instructions.push_back(i);
    }

    void emitLoopEnd(const std::string& label) {
        TACInstruction i; i.isLoopEnd = true;
        i.result = label; i.raw = "LOOP_END " + label;
        instructions.push_back(i);
    }

    // --- parse a simple expression tree into TAC, return result var ---
    // Supported grammar (very simplified, no real parse tree):
    //   expr := term (('+' | '-') term)*
    //   term := factor (('*' | '/') factor)*
    //   factor := NUMBER | IDENT | '(' expr ')'

    std::string parseExpr(const std::vector<Token>& tokens, size_t& pos);
    std::string parseTerm(const std::vector<Token>& tokens, size_t& pos);
    std::string parseFactor(const std::vector<Token>& tokens, size_t& pos);

    // Parse a full source program (very simplified) into TAC
    void generateIR(const std::string& source);
};

std::string IRGenerator::parseFactor(const std::vector<Token>& toks, size_t& pos) {
    if (pos < toks.size() && toks[pos].type == "PAREN" && toks[pos].value == "(") {
        ++pos;
        std::string r = parseExpr(toks, pos);
        if (pos < toks.size() && toks[pos].value == ")") ++pos;
        return r;
    }
    if (pos < toks.size() &&
        (toks[pos].type == "NUMBER" || toks[pos].type == "IDENT")) {
        return toks[pos++].value;
    }
    return "0";
}

std::string IRGenerator::parseTerm(const std::vector<Token>& toks, size_t& pos) {
    std::string left = parseFactor(toks, pos);
    while (pos < toks.size() && toks[pos].type == "OP" &&
           (toks[pos].value == "*" || toks[pos].value == "/")) {
        std::string op = toks[pos++].value;
        std::string right = parseFactor(toks, pos);
        std::string tmp = newTemp();
        emit(tmp, op, left, right);
        left = tmp;
    }
    return left;
}

std::string IRGenerator::parseExpr(const std::vector<Token>& toks, size_t& pos) {
    std::string left = parseTerm(toks, pos);
    while (pos < toks.size() && toks[pos].type == "OP" &&
           (toks[pos].value == "+" || toks[pos].value == "-")) {
        std::string op = toks[pos++].value;
        std::string right = parseTerm(toks, pos);
        std::string tmp = newTemp();
        emit(tmp, op, left, right);
        left = tmp;
    }
    return left;
}

void IRGenerator::generateIR(const std::string& source) {
    Lexer lexer;
    auto tokens = lexer.tokenize(source);
    size_t pos = 0;
    int loopCount = 0;

    while (pos < tokens.size()) {
        // --- skip type keywords (int, float, void) ---
        if (tokens[pos].type == "KEYWORD" &&
            (tokens[pos].value == "int" || tokens[pos].value == "float" ||
             tokens[pos].value == "void")) {
            ++pos;
            // check: function definition? (ident followed by '(')
            if (pos + 1 < tokens.size() && tokens[pos].type == "IDENT" &&
                tokens[pos+1].value == "(") {
                ++pos; // skip function name
                // skip parameter list
                while (pos < tokens.size() && tokens[pos].value != ")") ++pos;
                if (pos < tokens.size()) ++pos; // consume ')'
                continue;
            }
            // else: local variable declaration -- fall through to assignment handling
            // (pos now points at the variable identifier)
            continue;  // re-enter loop with pos at IDENT
        }

        // --- return statement ---
        if (tokens[pos].type == "KEYWORD" && tokens[pos].value == "return") {
            ++pos;
            if (pos < tokens.size() && tokens[pos].type != "SEMI") {
                std::string val = parseExpr(tokens, pos);
                emitReturn(val);
            } else {
                emitReturn("");
            }
            if (pos < tokens.size() && tokens[pos].type == "SEMI") ++pos;
            continue;
        }

        // --- for loop: for(i=0; i<N; i++) ---
        if (tokens[pos].type == "KEYWORD" && tokens[pos].value == "for") {
            ++pos; // skip 'for'
            if (pos < tokens.size() && tokens[pos].value == "(") ++pos;

            std::string loopLabel = "LOOP_" + std::to_string(++loopCount);
            emitLoopHead(loopLabel);

            // init: i = 0
            if (pos < tokens.size() && tokens[pos].type == "IDENT") {
                std::string var = tokens[pos++].value;
                if (pos < tokens.size() && tokens[pos].type == "ASSIGN") ++pos;
                std::string val = parseExpr(tokens, pos);
                emit(var, "", val);
                if (pos < tokens.size() && tokens[pos].type == "SEMI") ++pos;
            }
            // cond: skip for simplicity, consume to next ;
            while (pos < tokens.size() && tokens[pos].type != "SEMI") ++pos;
            if (pos < tokens.size()) ++pos;
            // increment: skip to )
            while (pos < tokens.size() && tokens[pos].value != ")") ++pos;
            if (pos < tokens.size()) ++pos;
            // skip '{' '}'
            if (pos < tokens.size() && tokens[pos].value == "{") ++pos;
            continue;
        }

        // --- end brace (close loop/block) ---
        if (tokens[pos].type == "BRACE" && tokens[pos].value == "}") {
            if (loopCount > 0) {
                emitLoopEnd("LOOP_" + std::to_string(loopCount--));
            }
            ++pos;
            continue;
        }

        // --- assignment: IDENT = expr ; ---
        if (tokens[pos].type == "IDENT") {
            std::string var = tokens[pos++].value;
            if (pos < tokens.size() && tokens[pos].type == "ASSIGN") {
                ++pos;
                std::string rhs = parseExpr(tokens, pos);
                emit(var, "", rhs);
            }
            if (pos < tokens.size() && tokens[pos].type == "SEMI") ++pos;
            continue;
        }

        ++pos; // skip anything unrecognised
    }
}

void printIR(const std::vector<TACInstruction>& ir, const std::string& title) {
    std::cout << "\n=== " << title << " ===\n";
    for (auto& ins : ir)
        std::cout << "  " << ins.raw << "\n";
    std::cout << "\n";
}

// ============================================================
// ============= TASK 3: OPTIMIZATION PASSES ==================
// ============================================================

class Optimizer {
public:
    // ---- 3a. Constant Folding ----
    // If arg1 and arg2 are both numeric constants and op is arithmetic,
    // replace the instruction with a direct assignment.
    static bool isNumber(const std::string& s) {
        if (s.empty()) return false;
        size_t start = (s[0] == '-') ? 1 : 0;
        for (size_t i = start; i < s.size(); ++i)
            if (!std::isdigit(s[i])) return false;
        return s.size() > start;
    }

    static std::vector<TACInstruction> constantFolding(
            std::vector<TACInstruction> ir) {
        for (auto& ins : ir) {
            if (ins.isLabel || ins.isJump || ins.isReturn ||
                ins.isLoopHead || ins.isLoopEnd) continue;
            if (!ins.op.empty() && isNumber(ins.arg1) && isNumber(ins.arg2)) {
                long long a = std::stoll(ins.arg1);
                long long b = std::stoll(ins.arg2);
                long long res = 0;
                bool ok = true;
                if (ins.op == "+") res = a + b;
                else if (ins.op == "-") res = a - b;
                else if (ins.op == "*") res = a * b;
                else if (ins.op == "/" && b != 0) res = a / b;
                else ok = false;
                if (ok) {
                    // Fold: t1 = 3 + 4  =>  t1 = 7
                    ins.arg1 = std::to_string(res);
                    ins.op   = "";
                    ins.arg2 = "";
                    ins.raw  = ins.result + " = " + ins.arg1;
                    // [Constant Folding] applied
                }
            }
        }
        return ir;
    }

    // ---- 3b. Constant Propagation ----
    // Track variables holding constant values, substitute them.
    static std::vector<TACInstruction> constantPropagation(
            std::vector<TACInstruction> ir) {
        std::map<std::string, std::string> constMap;
        for (auto& ins : ir) {
            if (ins.isLabel || ins.isJump || ins.isReturn ||
                ins.isLoopHead || ins.isLoopEnd) continue;

            // Substitute known constants
            if (constMap.count(ins.arg1)) ins.arg1 = constMap[ins.arg1];
            if (constMap.count(ins.arg2)) ins.arg2 = constMap[ins.arg2];

            // Record if result is assigned a constant
            if (ins.op.empty() && isNumber(ins.arg1))
                constMap[ins.result] = ins.arg1;
            else
                constMap.erase(ins.result); // no longer a known constant

            // Rebuild raw
            ins.raw = ins.result + " = " + ins.arg1;
            if (!ins.op.empty()) ins.raw += " " + ins.op + " " + ins.arg2;
        }
        return ir;
    }

    // ---- 3c. Algebraic Simplification ----
    // x * 1 => x,  x + 0 => x,  x * 0 => 0, etc.
    static std::vector<TACInstruction> algebraicSimplification(
            std::vector<TACInstruction> ir) {
        for (auto& ins : ir) {
            if (ins.isLabel || ins.isJump || ins.isReturn ||
                ins.isLoopHead || ins.isLoopEnd || ins.op.empty()) continue;

            bool simplified = false;
            std::string newArg = "";

            if (ins.op == "*") {
                if (ins.arg2 == "1") { newArg = ins.arg1; simplified = true; }
                else if (ins.arg1 == "1") { newArg = ins.arg2; simplified = true; }
                else if (ins.arg2 == "0" || ins.arg1 == "0") {
                    newArg = "0"; simplified = true;
                }
            } else if (ins.op == "+") {
                if (ins.arg2 == "0") { newArg = ins.arg1; simplified = true; }
                else if (ins.arg1 == "0") { newArg = ins.arg2; simplified = true; }
            } else if (ins.op == "-") {
                if (ins.arg2 == "0") { newArg = ins.arg1; simplified = true; }
            } else if (ins.op == "/") {
                if (ins.arg2 == "1") { newArg = ins.arg1; simplified = true; }
            }

            if (simplified) {
                ins.arg1 = newArg;
                ins.op   = "";
                ins.arg2 = "";
                ins.raw  = ins.result + " = " + ins.arg1;
                // [Algebraic Simplification] applied
            }
        }
        return ir;
    }

    // ---- 3d. Common Subexpression Elimination (CSE) ----
    // If the same (arg1 op arg2) was already computed, reuse the temp.
    static std::vector<TACInstruction> cse(std::vector<TACInstruction> ir) {
        // Map "arg1 op arg2" -> result temp
        std::map<std::string, std::string> exprMap;
        for (auto& ins : ir) {
            if (ins.isLabel || ins.isJump || ins.isReturn ||
                ins.isLoopHead || ins.isLoopEnd || ins.op.empty()) continue;

            std::string key = ins.arg1 + " " + ins.op + " " + ins.arg2;
            if (exprMap.count(key)) {
                // Replace with copy of existing temp
                ins.arg1 = exprMap[key];
                ins.op   = "";
                ins.arg2 = "";
                ins.raw  = ins.result + " = " + ins.arg1;
                // [CSE] applied
            } else {
                exprMap[key] = ins.result;
            }
        }
        return ir;
    }

    // ---- Combined pass ----
    static std::vector<TACInstruction> applyAll(
            std::vector<TACInstruction> ir,
            bool verbose = true) {
        if (verbose) std::cout << "[Optimizer] Applying Constant Folding...\n";
        ir = constantFolding(ir);
        if (verbose) std::cout << "[Optimizer] Applying Constant Propagation...\n";
        ir = constantPropagation(ir);
        if (verbose) std::cout << "[Optimizer] Applying Algebraic Simplification...\n";
        ir = algebraicSimplification(ir);
        if (verbose) std::cout << "[Optimizer] Applying CSE...\n";
        ir = cse(ir);
        return ir;
    }
};

// ============================================================
// ====== TASK 4: DEAD/UNREACHABLE CODE ELIMINATION ===========
// ============================================================

class DeadCodeEliminator {
public:
    // After a 'return' statement, all following instructions until next label
    // are unreachable.
    static std::vector<TACInstruction> eliminate(
            std::vector<TACInstruction> ir) {
        std::vector<TACInstruction> result;
        bool unreachable = false;
        for (auto& ins : ir) {
            if (ins.isLabel) {
                unreachable = false; // label resets reachability
                result.push_back(ins);
                continue;
            }
            if (unreachable) {
                std::cout << "  [Dead Code Eliminated]: " << ins.raw << "\n";
                continue; // drop instruction
            }
            result.push_back(ins);
            if (ins.isReturn) unreachable = true;
        }
        return result;
    }

    // Dead variable elimination: variables assigned but never used
    static std::vector<TACInstruction> deadVarElimination(
            std::vector<TACInstruction> ir) {
        // Count uses of each variable
        std::map<std::string, int> useCount;
        for (auto& ins : ir) {
            if (!ins.arg1.empty() && !Optimizer::isNumber(ins.arg1))
                useCount[ins.arg1]++;
            if (!ins.arg2.empty() && !Optimizer::isNumber(ins.arg2))
                useCount[ins.arg2]++;
            if (ins.isReturn && !ins.arg1.empty())
                useCount[ins.arg1]++;
        }
        std::vector<TACInstruction> result;
        for (auto& ins : ir) {
            if (!ins.isLabel && !ins.isJump && !ins.isReturn &&
                !ins.isLoopHead && !ins.isLoopEnd) {
                // If result is a temp var and never used, skip
                if (!ins.result.empty() && ins.result[0] == 't' &&
                    useCount[ins.result] == 0) {
                    std::cout << "  [Dead Var Eliminated]: " << ins.raw << "\n";
                    continue;
                }
            }
            result.push_back(ins);
        }
        return result;
    }
};

// ============================================================
// ============= TASK 5: LOOP OPTIMIZATIONS ===================
// ============================================================

class LoopOptimizer {
public:
    // ---- 5a. Loop Invariant Code Motion (LICM) ----
    // Move instructions inside a loop whose operands are not modified
    // inside the loop to before the loop.
    static std::vector<TACInstruction> licm(std::vector<TACInstruction> ir) {
        std::vector<TACInstruction> result;
        size_t i = 0;
        while (i < ir.size()) {
            if (!ir[i].isLoopHead) { result.push_back(ir[i++]); continue; }

            // Collect the loop body (between LOOP_BEGIN and LOOP_END)
            std::string loopLabel = ir[i].result;
            std::vector<TACInstruction> loopBody;
            size_t j = i + 1;
            while (j < ir.size() && !ir[j].isLoopEnd) loopBody.push_back(ir[j++]);

            // Find variables modified inside loop
            std::set<std::string> modified;
            for (auto& ins : loopBody)
                if (!ins.result.empty()) modified.insert(ins.result);

            // Hoist invariant instructions
            std::vector<TACInstruction> hoisted, remaining;
            for (auto& ins : loopBody) {
                bool inv = !ins.isLabel && !ins.isJump && !ins.op.empty() &&
                           !modified.count(ins.arg1) &&
                           (ins.arg2.empty() || !modified.count(ins.arg2));
                if (inv) {
                    std::cout << "  [LICM Hoisted]: " << ins.raw << "\n";
                    hoisted.push_back(ins);
                    modified.erase(ins.result);
                } else {
                    remaining.push_back(ins);
                }
            }

            // Emit: hoisted code, then loop head, then remaining body, then loop end
            for (auto& h : hoisted) result.push_back(h);
            result.push_back(ir[i]); // LOOP_BEGIN
            for (auto& r : remaining) result.push_back(r);
            if (j < ir.size()) result.push_back(ir[j]); // LOOP_END
            i = j + 1;
        }
        return result;
    }

    // ---- 5b. Loop Fusion ----
    // Merge two adjacent loops with the same trip count into one.
    // Condition: both loops iterate over the same range (same init / cond).
    static std::vector<TACInstruction> loopFusion(
            std::vector<TACInstruction> ir) {
        // Simple heuristic: find two consecutive LOOP_BEGIN..LOOP_END pairs
        // that share the same induction variable initialisation right before them.
        // We detect this by looking for back-to-back loop regions.
        std::vector<TACInstruction> result;
        size_t i = 0;
        while (i < ir.size()) {
            if (!ir[i].isLoopHead) { result.push_back(ir[i++]); continue; }

            // First loop
            size_t start1 = i;
            std::vector<TACInstruction> body1;
            size_t j = i + 1;
            while (j < ir.size() && !ir[j].isLoopEnd) body1.push_back(ir[j++]);
            size_t end1 = j;

            // Check for second consecutive loop
            if (end1 + 1 < ir.size() && ir[end1 + 1].isLoopHead) {
                size_t start2 = end1 + 1;
                std::vector<TACInstruction> body2;
                size_t k = start2 + 1;
                while (k < ir.size() && !ir[k].isLoopEnd) body2.push_back(ir[k++]);
                size_t end2 = k;

                std::cout << "  [Loop Fusion]: Merging "
                          << ir[start1].result << " and "
                          << ir[start2].result << "\n";

                // Emit fused loop
                TACInstruction head = ir[start1];
                head.raw = "LOOP_BEGIN " + head.result + "_FUSED";
                result.push_back(head);
                for (auto& b : body1) result.push_back(b);
                for (auto& b : body2) result.push_back(b);
                TACInstruction tail = ir[end1];
                tail.raw = "LOOP_END " + head.result + "_FUSED";
                result.push_back(tail);

                i = end2 + 1;
                continue;
            }

            // No fusion possible, emit as-is
            result.push_back(ir[start1]);
            for (auto& b : body1) result.push_back(b);
            if (end1 < ir.size()) result.push_back(ir[end1]);
            i = end1 + 1;
        }
        return result;
    }

    // ---- 5c. Strength Reduction ----
    // Replace multiplications by induction variable with additions.
    // Pattern: t = i * c  =>  keep an accumulator that adds c each iteration.
    static std::vector<TACInstruction> strengthReduction(
            std::vector<TACInstruction> ir) {
        for (auto& ins : ir) {
            if (ins.op == "*" && Optimizer::isNumber(ins.arg2)) {
                long long c = std::stoll(ins.arg2);
                if (c == 2) {
                    // Replace x * 2 with x + x (addition is cheaper)
                    ins.op   = "+";
                    ins.arg2 = ins.arg1;
                    ins.raw  = ins.result + " = " + ins.arg1 + " + " + ins.arg2;
                    std::cout << "  [Strength Reduction]: mul->add for " << ins.result << "\n";
                }
            }
        }
        return ir;
    }
};

// ============================================================
// ===== TASK 6: EXECUTION TIME COMPARISON ====================
// ============================================================

void benchmarkComparison() {
    std::cout << "\n=== TASK 6: EXECUTION TIME COMPARISON ===\n";
    const int RUNS  = 5;
    const int N     = 50000000;

    // --- Unoptimized: recompute 2*i every iteration ---
    long long totalUnopt = 0;
    double sumUnopt = 0;
    for (int r = 0; r < RUNS; ++r) {
        auto t0 = std::chrono::high_resolution_clock::now();
        volatile long long sum = 0;
        for (int i = 0; i < N; ++i) {
            int factor = 2;            // computed each iteration (unoptimized)
            sum += i * factor;
        }
        totalUnopt = sum;
        auto t1 = std::chrono::high_resolution_clock::now();
        sumUnopt += std::chrono::duration<double, std::milli>(t1 - t0).count();
    }

    // --- Optimized: strength reduction (addition instead of multiply) ---
    long long totalOpt = 0;
    double sumOpt = 0;
    for (int r = 0; r < RUNS; ++r) {
        auto t0 = std::chrono::high_resolution_clock::now();
        volatile long long sum = 0;
        long long acc = 0;
        for (int i = 0; i < N; ++i) {
            sum += acc;               // strength-reduced: add instead of mul
            acc += 2;
        }
        totalOpt = sum;
        auto t1 = std::chrono::high_resolution_clock::now();
        sumOpt += std::chrono::duration<double, std::milli>(t1 - t0).count();
    }

    double avgUnopt = sumUnopt / RUNS;
    double avgOpt   = sumOpt   / RUNS;
    double speedup  = avgUnopt / avgOpt;

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "  Unoptimized avg time : " << avgUnopt  << " ms\n";
    std::cout << "  Optimized   avg time : " << avgOpt    << " ms\n";
    std::cout << "  Speedup              : " << speedup   << "x\n";
    std::cout << "  (Both produce sum = " << totalOpt << ")\n";
}

// ============================================================
// ========================= MAIN =============================
// ============================================================

int main() {
    std::cout << "====================================================\n";
    std::cout << " CS-346 Lab 11 : Code & Loop Optimization\n";
    std::cout << " GitHub: https://github.com/Abrarbut/compiler-construction\n";
    std::cout << "====================================================\n";

    // -------- Sample source program --------
    std::string source = R"(
int compute(int a) {
    int x = 3 + 4;
    int y = x * 1;
    int z = y + 0;
    int w = z * 2;
    int t = z * 2;
    return t;
    int dead = 99;
}
)";

    std::string loopSource = R"(
int loopFunc() {
    int base = 10 * 2;
    for (i = 0; i < 100; i++) {
        sum = sum + i;
        stride = base * 3;
    }
    for (j = 0; j < 100; j++) {
        total = total + j;
    }
    return sum;
}
)";

    // ---- Lexer ----
    Lexer lexer;
    auto tokens = lexer.tokenize(source);
    lexer.printTokens(tokens);

    // ---- Task 2: IR Generation ----
    IRGenerator gen;
    gen.generateIR(source);
    printIR(gen.instructions, "TASK 2: UNOPTIMIZED IR");

    // ---- Task 3: Optimizations ----
    auto opt = Optimizer::applyAll(gen.instructions);
    printIR(opt, "TASK 3: IR AFTER OPTIMIZATION (CF + CP + AS + CSE)");

    // ---- Task 4: Dead/Unreachable Code Elimination ----
    std::cout << "\n=== TASK 4: DEAD CODE ELIMINATION ===\n";
    auto clean = DeadCodeEliminator::eliminate(opt);
    clean = DeadCodeEliminator::deadVarElimination(clean);
    printIR(clean, "TASK 4: IR AFTER DEAD CODE ELIMINATION");

    // ---- Task 5: Loop Optimizations ----
    IRGenerator loopGen;
    loopGen.generateIR(loopSource);
    printIR(loopGen.instructions, "TASK 5: LOOP IR (BEFORE OPTIMIZATION)");

    std::cout << "\n=== TASK 5a: LOOP INVARIANT CODE MOTION ===\n";
    auto loopOpt = LoopOptimizer::licm(loopGen.instructions);

    std::cout << "\n=== TASK 5b: LOOP FUSION ===\n";
    loopOpt = LoopOptimizer::loopFusion(loopOpt);

    std::cout << "\n=== TASK 5c: STRENGTH REDUCTION ===\n";
    loopOpt = LoopOptimizer::strengthReduction(loopOpt);

    printIR(loopOpt, "TASK 5: LOOP IR (AFTER OPTIMIZATION)");

    // ---- Task 6: Benchmarks ----
    benchmarkComparison();

    std::cout << "\n====================================================\n";
    std::cout << " Lab 11 Complete.\n";
    std::cout << "====================================================\n";
    return 0;
}