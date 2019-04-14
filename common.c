#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "common.h"

token_t *tokens;
token_t *current_token;

char *id_to_str(int id)
{
    char *code;
    switch(id) {
        case(ID):
            code = "ID";
            break;
        case(BREAK):
            code = "BREAK";
            break;
        case(CHAR):
            code = "CHAR";
            break;
        case(DOUBLE):
            code = "DOUBLE";
            break;
        case(ELSE):
            code = "ELSE";
            break;
        case(FOR):
             code = "FOR";
            break;
        case(IF):
            code = "IF";
            break;
        case(INT):
            code = "INT";
            break;
        case(RETURN):
            code = "RETURN";
            break;
        case(STRUCT):
            code = "STRUCT";
            break;
        case(VOID):
            code = "VOID";
            break;
        case(WHILE):
            code = "WHILE";
            break;
        case(END):
            code = "END";
            break;
        case(CT_INT):
            code = "CT_INT";
            break;
      case(CT_REAL):
            code = "CT_REAL";
            break;
        case(CT_CHAR):
            code = "CT_CHAR";
            break;
        case(CT_STRING):
            code = "CT_STRING";
            break;
        case(COMMA):
            code = "COMMA";
            break;
        case(SEMICOLON):
            code = "SEMICOLON";
            break;
        case(LPAR):
            code = "LPAR";
            break;
        case(RPAR):
            code = "RPAR";
            break;
        case(LBRACKET):
            code = "LBRACKET";
            break;
        case(RBRACKET):
            code = "RBRACKET";
            break;
        case(LACC):
            code = "LACC";
            break;
        case(RACC):
            code = "RACC";
            break;
        case(ADD):
            code = "ADD";
            break;
        case(SUB):
            code = "SUB";
            break;
        case(MUL):
           code = "MUL";
            break;
        case(DIV):
            code = "DIV";
            break;
        case(DOT):
            code = "DOT";
            break;
        case(AND):
            code = "AND";
            break;
        case(OR):
            code = "OR";
            break;
        case(NOT):
            code = "NOT";
            break;
        case(ASSIGN):
            code = "ASSIGN";
            break;
        case(EQUAL):
            code = "EQUAL";
            break;
        case(NOTEQ):
            code = "NOTEQ";
            break;
        case(LESS):
            code = "LESS";
            break;
        case(LESSEQ):
            code = "LESSEQ";
            break;
        case(GREATER):
            code = "GREATER";
            break;
        case(GREATEREQ):
            code = "GREATEREQ";
            break;

        default:
            code = "ERROR";
            break;
    }

    return code;
}

void err(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    fprintf(stderr, "Error: ");
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);

    va_end(va);
    exit(1);
}

void tkerr(const token_t *tk, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    fprintf(stderr, "Error in line %d: ", tk->line);
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);

    va_end(va);
    exit(1);

}

