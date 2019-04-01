#ifndef _H_LEXICAL_ANALYZER
#define _H_LEXICAL_ANALYZER

#include "common.h"

#define SAFEALLOC(var,type) if((var = (type*)malloc(sizeof(type))) == NULL) err("Not enough memory");

token *last_token;
char *p_crt_ch;
extern int line;

token *add_token(int code);
void print_atoms(token *);
void escape_char(char *);
int get_next_token();

#endif
