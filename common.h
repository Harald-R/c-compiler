#ifndef _H_COMMON
#define _H_COMMON

#include <stdlib.h>

#define SAFEALLOC(var,type) if((var = (type*)malloc(sizeof(type))) == NULL) err("Not enough memory");

enum {ID = 100,
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
} token_t;

extern token_t *tokens;
extern token_t *current_token;

char *id_to_str(int id);

void err(const char *fmt, ...);
void tkerr(const token_t *tk, const char *fmt, ...);

#endif
