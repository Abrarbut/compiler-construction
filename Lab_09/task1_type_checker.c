/*
 * Lab 09 - Task 1: Type Checking and Type Casting
 * Course: CS-346 Compiler Construction
 * NUST - SEECS
 *
 * Compile: gcc task1_type_checker.c -o task1
 * Run:     ./task1
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* -----------------------------------------
   Data-type enumeration & Symbol structure
   ----------------------------------------- */
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_UNKNOWN
} DataType;

typedef struct {
    char name[50];
    DataType type;
    union {
        int   ival;
        float fval;
        char  cval;
    } value;
} Symbol;

/* -----------------------------------------
   Helper: return printable type name
   ----------------------------------------- */
const char* typeName(DataType t) {
    switch (t) {
        case TYPE_INT:   return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_CHAR:  return "char";
        default:         return "unknown";
    }
}

/* -----------------------------------------
   Type Checking
   Returns 1 if assigning `rhs` type to `lhs`
   type is compatible, 0 otherwise.
   ----------------------------------------- */
int typeCheck(DataType lhs, DataType rhs) {
    if (lhs == rhs)                               return 1;  /* same type   */
    if (lhs == TYPE_INT   && rhs == TYPE_CHAR)    return 1;  /* char -> int */
    if (lhs == TYPE_FLOAT && rhs == TYPE_INT)     return 0;  /* needs cast  */
    if (lhs == TYPE_INT   && rhs == TYPE_FLOAT)   return 0;  /* needs cast  */
    return 0;
}

/* -----------------------------------------
   Type Casting
   Converts src to targetType and returns a
   new Symbol with the converted value.
   ----------------------------------------- */
Symbol typeCast(Symbol src, DataType targetType) {
    Symbol result;
    strcpy(result.name, src.name);
    result.type = targetType;

    if (src.type == TYPE_FLOAT && targetType == TYPE_INT) {
        result.value.ival = (int)src.value.fval;
        printf("  [Cast] float %.4f  -->  int %d  (fractional part truncated)\n",
               src.value.fval, result.value.ival);

    } else if (src.type == TYPE_INT && targetType == TYPE_FLOAT) {
        result.value.fval = (float)src.value.ival;
        printf("  [Cast] int %d  -->  float %.4f\n",
               src.value.ival, result.value.fval);

    } else if (src.type == TYPE_CHAR && targetType == TYPE_INT) {
        result.value.ival = (int)src.value.cval;
        printf("  [Cast] char '%c' (ASCII %d)  -->  int %d\n",
               src.value.cval, src.value.cval, result.value.ival);

    } else if (src.type == TYPE_INT && targetType == TYPE_CHAR) {
        result.value.cval = (char)src.value.ival;
        printf("  [Cast] int %d  -->  char '%c'\n",
               src.value.ival, result.value.cval);

    } else {
        printf("  [Cast Error] No valid cast from %s to %s\n",
               typeName(src.type), typeName(targetType));
        result = src;   /* keep original */
    }
    return result;
}

/* -----------------------------------------
   Print result of a type-check attempt
   ----------------------------------------- */
void reportTypeCheck(const char* lName, DataType lType,
                     const char* rName, DataType rType) {
    printf("  Assigning %-5s (%s) = %-5s (%s)  -->  ",
           lName, typeName(lType), rName, typeName(rType));
    if (typeCheck(lType, rType))
        printf("PASSED\n");
    else
        printf("TYPE ERROR: incompatible types (explicit cast required)\n");
}

/* -----------------------------------------
   Main
   ----------------------------------------- */
int main(void) {
    printf("+==========================================+\n");
    printf("|  Lab 09 - Task 1: Type Checking & Casting |\n");
    printf("+==========================================+\n\n");

    /* Declare sample symbols */
    Symbol varInt;   strcpy(varInt.name,   "a"); varInt.type   = TYPE_INT;   varInt.value.ival   = 42;
    Symbol varFloat; strcpy(varFloat.name, "b"); varFloat.type = TYPE_FLOAT; varFloat.value.fval = 9.81f;
    Symbol varChar;  strcpy(varChar.name,  "c"); varChar.type  = TYPE_CHAR;  varChar.value.cval  = 'Z';

    /* ---- Print symbol table ---- */
    printf("--- Symbol Table ---------------------------\n");
    printf("  %-6s  %-6s  %s\n", "Name", "Type", "Value");
    printf("  %-6s  %-6s  %d\n", varInt.name,   typeName(varInt.type),   varInt.value.ival);
    printf("  %-6s  %-6s  %.4f\n", varFloat.name, typeName(varFloat.type), varFloat.value.fval);
    printf("  %-6s  %-6s  '%c'\n",  varChar.name,  typeName(varChar.type),  varChar.value.cval);
    printf("\n");

    /* ---- TYPE CHECKING ---- */
    printf("--- Type Checking --------------------------\n");
    reportTypeCheck("a", TYPE_INT,   "a", TYPE_INT);       /* int  = int   -> OK          */
    reportTypeCheck("a", TYPE_INT,   "c", TYPE_CHAR);      /* int  = char  -> OK (implicit)*/
    reportTypeCheck("a", TYPE_INT,   "b", TYPE_FLOAT);     /* int  = float -> ERROR        */
    reportTypeCheck("b", TYPE_FLOAT, "a", TYPE_INT);       /* float= int   -> ERROR        */
    printf("\n");

    /* ---- TYPE CASTING ---- */
    printf("--- Type Casting ---------------------------\n");

    printf("(1) float -> int:\n");
    Symbol r1 = typeCast(varFloat, TYPE_INT);
    printf("      Result: %s %s = %d\n\n", typeName(r1.type), r1.name, r1.value.ival);

    printf("(2) int -> float:\n");
    Symbol r2 = typeCast(varInt, TYPE_FLOAT);
    printf("      Result: %s %s = %.4f\n\n", typeName(r2.type), r2.name, r2.value.fval);

    printf("(3) char -> int:\n");
    Symbol r3 = typeCast(varChar, TYPE_INT);
    printf("      Result: %s %s = %d\n\n", typeName(r3.type), r3.name, r3.value.ival);

    printf("(4) int -> char:\n");
    Symbol r4 = typeCast(varInt, TYPE_CHAR);
    printf("      Result: %s %s = '%c'\n\n", typeName(r4.type), r4.name, r4.value.cval);

    printf("--- End of Task 1 --------------------------\n");
    return 0;
}
