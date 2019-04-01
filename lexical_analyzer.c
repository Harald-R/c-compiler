#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "lexical_analyzer.h"

int line = 1;

token *add_token(int code)
{
    token *tk;
    SAFEALLOC(tk,token)

    tk->code = code;
    tk->line = line;
    tk->next = NULL;

    if (last_token) {
        last_token->next = tk;
    } else {
        tokens = tk;
    }
    last_token = tk;

    return tk;
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

void tkerr(const token *tk, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    fprintf(stderr, "Error in line %d: ", tk->line);
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);

    va_end(va);
    exit(1);

}

char *create_string(const char *p_start_ch, char *p_crt_ch)
{
    int size = p_crt_ch - p_start_ch;
    if (size < 0) {
        perror("Invalid string");
        exit(1);
    }

    char *string = (char *) malloc((size + 1) * sizeof(char));
    if (!string) {
        perror("Error allocating memory");
        exit(1);
    }

    strncpy(string, p_start_ch, size);
    string[size] = '\0';
    escape_char(string);

    return string;
}

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

void print_atoms(token *tokens)
{
    token *token = tokens;

    while (token != NULL) {
        if (token->code == ID || token->code == CT_STRING || token->code == CT_CHAR) {
            printf("%d %s:%s\n", token->line, id_to_str(token->code), token->text);
        } else if (token->code == CT_INT) {
            printf("%d %s:%ld\n", token->line, id_to_str(token->code), token->i);
        } else if (token->code == CT_REAL) {
            printf("%d %s:%g\n", token->line, id_to_str(token->code), token->r);
        } else {
            printf("%d %s\n", token->line, id_to_str(token->code));
        }

        token = token->next;
    }
}

void escape_char(char *str)
{
    
    unsigned long i = 1;
    while (i < strlen(str)) {
        if ((str[i] == '\\' || str[i] == '\'' ||
             str[i] == '\"' || str[i] == '\?' || 
             str[i] == 'a' || str[i] == 'b' || 
             str[i] == 'f' || str[i] == 'n' || 
             str[i] == 'r' || str[i] == 't' || 
             str[i] == 'v' || str[i] == '0') && 
             str[i-1] == '\\')
        {
            strcpy(str+i-1, str+i);
            // TODO
            //size_t len_right = strlen(str+i);
            //memmove(str+i-1, str+i, len_right);

            if (str[i-1] == 'a') str[i-1] = '\a';
            else if (str[i-1] == 'b') str[i-1] = '\b';
            else if (str[i-1] == 'f') str[i-1] = '\f';
            else if (str[i-1] == 'n') str[i-1] = '\n';
            else if (str[i-1] == 'r') str[i-1] = '\r';
            else if (str[i-1] == 't') str[i-1] = '\t';
            else if (str[i-1] == 'v') str[i-1] = '\v';
            else if (str[i-1] == '0') str[i-1] = '\0';
        }
        else
        {
            ++i;
        }
    }
}

int convert_to_int(char *str, int base)
{
    char *ptr;
    return strtol(str, &ptr, base);
}

int get_next_token()
{
    int state = 0;
    int num_ch;
    int base = 0;
    char ch;
    char *str;
    const char *p_start_ch;
    token *tk;

    while (1) {
        ch = *p_crt_ch;
        // printf("#%d %c(%d)\n", state, ch, ch);
        switch (state) {
            case 0:
                if (isalpha(ch) || ch == '_')
                {
                    p_start_ch = p_crt_ch;
                    ++p_crt_ch;
                    state = 1;
                }
                else if (ch == '0')
                {
                    p_start_ch = p_crt_ch;
                    ++p_crt_ch;
                    state = 4;
                    base = 8;
                }
                else if (ch >= '1' && ch <= '9')
                {
                    p_start_ch = p_crt_ch;
                    ++p_crt_ch;
                    state = 3;
                    base = 10;
                }
                else if (ch == '\'')
                {
                    p_start_ch = p_crt_ch;
                    ++p_crt_ch;
                    state = 16;
                }
                else if (ch == '\"')
                {
                    p_start_ch = p_crt_ch;
                    ++p_crt_ch;
                    state = 17;
                }
                else if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t')
                {
                    ++p_crt_ch;
                    if (ch == '\n') {
                        ++line;
                    }
                }
                else if (ch == '/')
                {
                    ++p_crt_ch;
                    state = 28;
                }
                else if (ch == ',')
                {
                    ++p_crt_ch;
                    state = COMMA;
                }
                else if (ch == ';')
                {
                    ++p_crt_ch;
                    state = SEMICOLON;
                }
                else if (ch == '(')
                {
                    ++p_crt_ch;
                    state = LPAR;
                }
                else if (ch == ')')
                {
                    ++p_crt_ch;
                    state = RPAR;
                }
                else if (ch == '[')
                {
                    ++p_crt_ch;
                    state = LBRACKET;
                }
                else if (ch == ']')
                {
                    ++p_crt_ch;
                    state = RBRACKET;
                }
                else if (ch == '{')
                {
                    ++p_crt_ch;
                    state = LACC;
                }
                else if (ch == '}')
                {
                    ++p_crt_ch;
                    state = RACC;
                }
                else if (ch == '+')
                {
                    ++p_crt_ch;
                    state = ADD;
                }
                else if (ch == '+')
                {
                    ++p_crt_ch;
                    state = ADD;
                }
                else if (ch == '-')
                {
                    ++p_crt_ch;
                    state = SUB;
                }
                else if (ch == '*')
                {
                    ++p_crt_ch;
                    state = MUL;
                }
                else if (ch == '&')
                {
                    ++p_crt_ch;
                    state = 46;
                }
                else if (ch == '|')
                {
                    ++p_crt_ch;
                    state = 48;
                }
                else if (ch == '.')
                {
                    ++p_crt_ch;
                    state = DOT;
                }
                else if (ch == '!')
                {
                    ++p_crt_ch;
                    state = 51;
                }
                else if (ch == '=')
                {
                    ++p_crt_ch;
                    state = 54;
                }
                else if (ch == '<')
                {
                    ++p_crt_ch;
                    state = 57;
                }
                else if (ch == '>')
                {
                    ++p_crt_ch;
                    state = 60;
                }
                else if (ch == '\0')
                {
                    ++p_crt_ch;
                    state = END;
                }
                else
                {
                    err("Invalid character: %c\n", ch);
                }

                break;

            case 1:
                if (isalnum(ch) || ch == '_')
                {
                    ++p_crt_ch;
                }
                else
                {
                    //++p_crt_ch;
                    state = ID;
                }
                break;

            case 3:
                if (ch >= '0' && ch <= '9')
                {
                    ++p_crt_ch;
                }
                else if (ch == '.')
                {
                    ++p_crt_ch;
                    state = 10;
                }
                else if (ch == 'e' || ch == 'E')
                {
                    ++p_crt_ch;
                    state = 11;
                }
                else
                {
                    //++p_crt_ch;
                    state = CT_INT;
                }
                break;

            case 4:
                if (ch >= '0' && ch <= '7')
                {
                    ++p_crt_ch;
                    state = 5;
                }
                else if (ch >= '8' && ch <= '9')
                {
                    ++p_crt_ch;
                    state = 9;
                    base = 10;
                }
                else if (ch == 'x')
                {
                    ++p_crt_ch;
                    state = 6;
                    base = 16;
                }
                else if (ch == '.')
                {
                    ++p_crt_ch;
                    state = 10;
                }
                else if (ch == 'e' || ch == 'E')
                {
                    ++p_crt_ch;
                    state = 10;
                }
                else
                {
                    //++p_crt_ch;
                    state = CT_INT;
                }
                break;

            case 5:
                if (ch >= '0' && ch <= '7')
                {
                    ++p_crt_ch;
                }
                else if (ch >= '8' && ch <= '9')
                {
                    ++p_crt_ch;
                    state = 9;
                    base = 10;
                }
                else if (ch == '.')
                {
                    ++p_crt_ch;
                    state = 10;
                }
                else if (ch == 'e' || ch == 'E')
                {
                    ++p_crt_ch;
                    state = 11;
                }
                else
                {
                    //++p_crt_ch;
                    state = CT_INT;
                }
                break;

            case 6:
                if (isxdigit(ch))
                {
                    ++p_crt_ch;
                    state = 7;
                }
                else
                {
                    err("Invalid character: %c\n", ch);
                }
                break;

            case 7:
                if (isxdigit(ch))
                {
                    ++p_crt_ch;
                }
                else
                {
                    //++p_crt_ch;
                    state = CT_INT;
                }
                break;

            case 9:
                if (isdigit(ch))
                {
                    ++p_crt_ch;
                }
                else if (ch == '.')
                {
                    ++p_crt_ch;
                    state = 10;
                }
                else if (ch == 'e' || ch == 'E')
                {
                    ++p_crt_ch;
                    state = 11;
                }
                else
                {
                    err("Invalid character: %c\n", ch);
                }
                break;

            case 10:
                if (isdigit(ch))
                {
                    ++p_crt_ch;
                    state = 12;
                }
                else
                {
                    err("Invalid character: %c\n", ch);
                }
                break;

            case 11:
                if (ch == '+' || ch == '-')
                {
                    ++p_crt_ch;
                }
                state = 13;
                break;

            case 12:
                if (isdigit(ch))
                {
                    ++p_crt_ch;
                }
                else if (ch == 'e' || ch == 'E')
                {
                    ++p_crt_ch;
                    state = 11;
                }
                else 
                {
                    //++p_crt_ch;
                    state = CT_REAL;
                }
                break;
 
            case 13:
                if (isdigit(ch))
                {
                    ++p_crt_ch;
                    state = 14;
                }
                else 
                {
                    err("Invalid character: %c\n", ch);
                }
                break;

            case 14:
                if (isdigit(ch))
                {
                    ++p_crt_ch;
                }
                else 
                {
                    //++p_crt_ch;
                    state = CT_REAL;
                }
                break;

            case 16:
                if (ch == '\\')
                {
                    ++p_crt_ch;
                    state = 19;
                }
                else if (ch != '\'' && ch != '\\')
                {
                    ++p_crt_ch;
                    state = 20;
                }
                else
                {
                    err("Invalid character: %c\n", ch);
                }
                break;

            case 17:
                if (ch == '\"')
                {
                    ++p_crt_ch;
                    state = CT_STRING;
                }
                else if (ch == '\\')
                {
                    ++p_crt_ch;
                    state = 24;
                }
                else
                {
                    ++p_crt_ch;
                    state = 23;
                }
                break;

            case 19:
                if (ch == 'a' || ch == 'b' || ch == 'f' || ch == 'r' || ch == 'n' ||
                   ch == 't' || ch == 'v' || ch == '\'' || ch == '\?' || ch == '\"' ||
                   ch == '\\' || ch == '\0')
                {
                    ++p_crt_ch;
                    state = 20;
                }
                else
                {
                    err("Invalid character: %c\n", ch);
                }
                break;

            case 20:
                if (ch == '\'')
                {
                    ++p_crt_ch;
                    state = CT_CHAR;
                }
                else
                {
                    err("Invalid character: %c\n", ch);
                }
                break;

            case 23:
                if (ch == '\\')
                {
                    ++p_crt_ch;
                    state = 24;
                }
                else if (ch == '\"')
                {
                    ++p_crt_ch;
                    state = CT_STRING;
                }
                else
                {
                    ++p_crt_ch;
                }
                break;

            case 24: 
                if (ch == 'a' || ch == 'b' || ch == 'f' || ch == 'r' || ch == 'n' ||
                   ch == 't' || ch == 'v' || ch == '\'' || ch == '\?' || ch == '\"' ||
                   ch == '\\' || ch == '\0')
                {
                    ++p_crt_ch;
                    state = 25;
                }
                else
                {
                    ++p_crt_ch;
                    state = 24;
                }
                break;

            case 25:
                if (ch == '\\')
                {
                    ++p_crt_ch;
                    state = 24;
                }
                else if (ch == '\"')
                {
                    ++p_crt_ch;
                    state = CT_STRING;
                }
                else
                {
                    ++p_crt_ch;
                    state = 23;
                }
                break;

            case 28:
                if (ch == '/')
                {
                    ++p_crt_ch;
                    state = 29;
                }
                else if (ch == '*')
                {
                    ++p_crt_ch;
                    state = 31;
                }
                else
                {
                    ++p_crt_ch;
                    state = DIV;
                }
                break;

            case 29:
                if (ch != '\n' && ch != '\r' && ch != '\0')
                {
                    ++p_crt_ch;
                }
                else
                {
                    ++p_crt_ch;
                    state = 0;
                    if (ch == '\n') {
                       ++line;
                    }
                }
                break;

            case 31:
                if (ch == '*')
                {
                    ++p_crt_ch;
                    state = 32;
                }
                else
                {
                    ++p_crt_ch;
                    if (ch == '\n') {
                        ++line;
                    }
                }
                break;

            case 32:
                if (ch == '*')
                {
                    ++p_crt_ch;
                }
                else if (ch == '/')
                {
                    ++p_crt_ch;
                    state = 0;
                }
                else
                {
                    ++p_crt_ch;
                    state = 31;
                    if (ch == '\n') {
                        ++line;
                    }
                }
                break;

            case 46:
                if (ch == '&')
                {
                    ++p_crt_ch;
                    state = AND;
                }
                else
                {
                    err("Invalid character: %c\n", ch);
                }
                break;

            case 48:
                if (ch == '|')
                {
                    ++p_crt_ch;
                    state = OR;
                }
                else
                {
                    err("Invalid character: %c\n", ch);
                }
                break;

            case 51:
                if (ch == '=')
                {
                    ++p_crt_ch;
                    state = NOTEQ;
                }
                else
                {
                    ++p_crt_ch;
                    state = NOT;
                }
                break;

            case 54:
                if (ch == '=')
                {
                    ++p_crt_ch;
                    state = EQUAL;
                }
                else
                {
                    ++p_crt_ch;
                    state = ASSIGN;
                }
                break;

            case 57:
                if (ch == '=')
                {
                    ++p_crt_ch;
                    state = LESSEQ;
                }
                else
                {
                    ++p_crt_ch;
                    state = LESS;
                }
                break;

            case 60:
                if (ch == '=')
                {
                    ++p_crt_ch;
                    state = GREATEREQ;
                }
                else
                {
                    ++p_crt_ch;
                    state = GREATER;
                }
                break;

            case ID:
                num_ch = p_crt_ch - p_start_ch;
                if (num_ch == 5 && memcmp(p_start_ch, "break", num_ch) == 0) tk = add_token(BREAK);
                else if (num_ch == 4 && memcmp(p_start_ch, "char", num_ch) == 0) tk = add_token(CHAR);
                else if (num_ch == 6 && memcmp(p_start_ch, "double", num_ch) == 0) tk = add_token(DOUBLE);
                else if (num_ch == 4 && memcmp(p_start_ch, "else", num_ch) == 0) tk = add_token(ELSE);
                else if (num_ch == 3 && memcmp(p_start_ch, "for", num_ch) == 0) tk = add_token(FOR);
                else if (num_ch == 2 && memcmp(p_start_ch, "if", num_ch) == 0) tk = add_token(IF);
                else if (num_ch == 3 && memcmp(p_start_ch, "int", num_ch) == 0) tk = add_token(INT);
                else if (num_ch == 6 && memcmp(p_start_ch, "return", num_ch) == 0) tk = add_token(RETURN);
                else if (num_ch == 6 && memcmp(p_start_ch, "struct", num_ch) == 0) tk = add_token(STRUCT);
                else if (num_ch == 4 && memcmp(p_start_ch, "void", num_ch) == 0) tk = add_token(VOID);
                else if (num_ch == 5 && memcmp(p_start_ch, "while", num_ch) == 0) tk = add_token(WHILE);
                else {
                    tk = add_token(ID);
                    tk->text = create_string(p_start_ch, p_crt_ch);
                }
                return tk->code;

            case CT_INT:
                tk = add_token(CT_INT);
                
                str = create_string(p_start_ch, p_crt_ch);
                tk->i = convert_to_int(str, base);
                free(str);

                return tk->code;

            case CT_REAL:
                tk = add_token(CT_REAL);

                str = create_string(p_start_ch, p_crt_ch);
                tk->r = atof(str);
                free(str);

                return tk->code;

            case CT_CHAR:
                tk = add_token(CT_CHAR);
                tk->text = create_string(p_start_ch+1, p_crt_ch-1);
                return tk->code;

            case CT_STRING:
                tk = add_token(CT_STRING);
                tk->text = create_string(p_start_ch+1, p_crt_ch-1);
                return tk->code;

            case COMMA:
                tk = add_token(COMMA);
                return tk->code;

            case SEMICOLON:
                tk = add_token(SEMICOLON);
                return tk->code;

            case LPAR:
                tk = add_token(LPAR);
                return tk->code;

            case RPAR:
                tk = add_token(RPAR);
                return tk->code;

            case LBRACKET:
                tk = add_token(LBRACKET);
                return tk->code;

            case RBRACKET:
                tk = add_token(RBRACKET);
                return tk->code;

            case LACC:
                tk = add_token(LACC);
                return tk->code;

            case RACC:
                tk = add_token(RACC);
                return tk->code;

            case ADD:
                tk = add_token(ADD);
                return tk->code;

            case SUB:
                tk = add_token(SUB);
                return tk->code;

            case MUL:
                tk = add_token(MUL);
                return tk->code;

            case AND:
                tk = add_token(AND);
                return tk->code;

            case OR:
                tk = add_token(OR);
                return tk->code;

            case DOT:
                tk = add_token(DOT);
                return tk->code;

            case NOTEQ:
                tk = add_token(NOTEQ);
                return tk->code;

            case NOT:
                tk = add_token(NOT);
                return tk->code;

            case EQUAL:
                tk = add_token(EQUAL);
                return tk->code;

            case ASSIGN:
                tk = add_token(ASSIGN);
                return tk->code;

            case LESSEQ:
                tk = add_token(LESSEQ);
                return tk->code;

            case LESS:
                tk = add_token(LESS);
                return tk->code;

            case GREATEREQ:
                tk = add_token(GREATEREQ);
                return tk->code;

            case GREATER:
                tk = add_token(GREATER);
                return tk->code;

            case END:
                tk = add_token(END);
                return tk->code;

            default:
                err("Invalid state %d (%c)\n", state, ch);
        }
    }
}

void generate_tokens()
{
}
