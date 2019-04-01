#ifndef _H_LEXICAL_ANALYZER
#define _H_LEXICAL_ANALYZER

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#define SAFEALLOC(var,type) if((var = (type*)malloc(sizeof(type))) == NULL) err("Not enough memory");

enum{ID = 100,
    BREAK,
    CHAR,
    DOUBLE,
    ELSE,
    FOR,
    IF,
    INT,
    RETURN,
    STRUCT,
    VOID,
    WHILE,
    END,
    CT_INT,
    CT_REAL,
    CT_CHAR,
    CT_STRING,
    COMMA,
    SEMICOLON,
    LPAR,
    RPAR,
    LBRACKET,
    RBRACKET,
    LACC,
    RACC,
    ADD,
    SUB,
    MUL,
    DIV,
    DOT,
    AND,
    OR,
    NOT,
    ASSIGN,
    EQUAL,
    NOTEQ,
    LESS,
    LESSEQ,
    GREATER,
    GREATEREQ
    };

typedef struct _token {
    int code;
    union {
        char *text;
        long int i;
        double r;
    };
    int line;
    struct _token *next;
} token;

token *tokens;
token *last_token;
char *p_crt_ch;
extern int line;

token *add_token(int code);
void err(const char *fmt, ...);
void tkerr(const token *tk, const char *fmt, ...);
void print_atoms(token *);
void escape_char(char *);
int get_next_token();

#endif
