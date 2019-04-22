#ifndef _H_SYNTAX_ANALYZER
#define _H_SYNTAX_ANALYZER

#include "common.h"

#if 0
#define DEBUG_PRINTF(...) printf(__VA_ARGS__);
#else
#define DEBUG_PRINTF(...)
#endif

#define ERR_MISSING_STMT(x) "missing " #x " statement"
#define ERR_MISSING_LPAR(x) "missing ( after " #x
#define ERR_MISSING_RPAR "missing )"
#define ERR_INVALID_EXPR "invalid expression after )"

int analyze_syntax();

int consume(int code);

int unit();

int declStruct();
int declFunc();
int declVar();

int typeBase();
int typeName();
int arrayDecl();
int funcArg();

int stm();
int stmCompound();

int expr();
int exprAssign();
int exprOr();
int exprAnd();
int exprEq();
int exprRel();
int exprAdd();
int exprMul();
int exprCast();
int exprUnary();
int exprPostfix();
int exprPrimary();

#endif
