#ifndef _H_LEXICAL_ANALYZER
#define _H_LEXICAL_ANALYZER

#include "common.h"

token *last_token;
char *p_crt_ch;
extern int line;

token *add_token(int code);
void print_atoms(token *);
void escape_char(char *);
int get_next_token();

#endif
