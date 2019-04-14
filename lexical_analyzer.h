#ifndef _H_LEXICAL_ANALYZER
#define _H_LEXICAL_ANALYZER

#include "common.h"

void get_atoms(char *buffer);
void print_atoms(token_t *);

token_t *add_token(int code);
void escape_char(char *);
int get_next_token();

#endif
