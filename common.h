#ifndef _H_COMMON
#define _H_COMMON

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
} token;

token *tokens;

char *id_to_str(int id);

void err(const char *fmt, ...);
void tkerr(const token *tk, const char *fmt, ...);

#endif
