#ifndef _H_SYNTAX_ANALYZER
#define _H_SYNTAX_ANALYZER

#include "common.h"

#define ERR_MISSING_STMT(x) "missing " #x " statement"
#define ERR_MISSING_LPAR(x) "missing ( after " #x
#define ERR_MISSING_RPAR "missing )"
#define ERR_INVALID_EXPR "invalid expression after )"

token *current_token;

int consume(int code);

int unit();
int expr();
int stmt();
int rule_while();

#endif
