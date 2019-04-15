#include "symbol_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

symbols_t symbols;
int current_depth;

symbol_t *current_struct;
symbol_t *current_func;

void init_symbols(symbols_t *symbols)
{
    symbols->begin = NULL;
    symbols->end = NULL;
    symbols->after = NULL;
}

symbol_t *add_symbol(symbols_t *symbols, const char *name, int cls)
{
    symbol_t *s;

    if (symbols->end == symbols->after) {
        int size = symbols->after - symbols->begin;
        int new_size = size * 2;
        if (new_size == 0)
            new_size = 1;

        symbols->begin = (symbol_t **) realloc(symbols->begin, new_size * sizeof(symbol_t *));
        if (symbols->begin == NULL) {
            err("Not enough memory");
        }
        symbols->end   = symbols->begin + size;
        symbols->after = symbols->begin + new_size;
    }

    SAFEALLOC(s, symbol_t);
    *symbols->end = s;
    ++symbols->end;

    s->name = name;
    s->cls = cls;
    s->depth = current_depth;

    return s;
}

symbol_t *find_symbol(symbols_t *symbols, const char *name)
{
    symbol_t *s;
    int n = symbols->end - symbols->begin;
    
    for (int i = n-1; i >= 0; --i) {
        s = symbols->begin[i];
        if (strcmp(s->name, name) == 0)
            return s;
    }

    return NULL;
}

void add_var(token_t *token, type_t *type)
{
    symbol_t *s;

    if (current_struct) {
        if (find_symbol(&current_struct->members, token->text)) {
            tkerr(current_token, "symbol redefinition: %s", token->text);
        }
        s = add_symbol(&current_struct->members, token->text, CLS_VAR);
    }
    else if(current_func) {
        s = find_symbol(&symbols, token->text);
        if (s && s->depth == current_depth) {
            tkerr(current_token, "symbol redefinition: %s", token->text);
        }
        s = add_symbol(&symbols, token->text, CLS_VAR);
        s->mem = MEM_LOCAL;
    }
    else {
        if (find_symbol(&symbols, token->text)) {
            tkerr(current_token, "symbol redefinition: %s", token->text);
        }
        s = add_symbol(&symbols, token->text, CLS_VAR);
        s->mem = MEM_GLOBAL;
    }
    
    s->type = *type;
}

void delete_symbols_after(symbols_t *symbols, symbol_t *start)
{
    int i = 0;
    int n = symbols->end - symbols->begin;

    // Check if symbol is in table
    while (i < n && symbols->begin[i] != start) ++i;
    if (i == n) return;

    // Delete elements found after the given symbol
    for (; n > i; --n) {
        free(symbols->end[-1]);
        symbols->end--;
    }
}

void print_symbol_table(symbols_t *symbols)
{
    for (symbol_t **s = symbols->begin; s != symbols->end; ++s) {
        
        const char *str_type;
        const char *str_cls;
        const char *str_mem;

        switch ((*s)->cls) {
            case CLS_VAR:     str_cls = "VAR"; break;
            case CLS_FUNC:    str_cls = "FUNC"; break;
            case CLS_EXTFUNC: str_cls = "EXTFUNC"; break;
            case CLS_STRUCT:  str_cls = "STRUCT"; break;
            default:          str_cls = "UNKNOWN"; break;
        }

        if ((*s)->cls != CLS_STRUCT) {
            switch ((*s)->type.type_base) {
                case TB_INT:    str_type = "INT"; break;
                case TB_DOUBLE: str_type = "DOUBLE"; break;
                case TB_CHAR:   str_type = "CHAR"; break;
                case TB_STRUCT: str_type = "STRUCT"; break;
                case TB_VOID:   str_type = "VOID"; break;
                default:        str_type = "UNKNOWN"; break;
            }
        } else {
            str_type = "SYMBOL";
        }

        if ((*s)->cls == CLS_VAR) {
            switch ((*s)->mem) {
                case MEM_LOCAL:  str_mem = "LOCAL"; break;
                case MEM_ARG:    str_mem = "ARG"; break;
                case MEM_GLOBAL: str_mem = "GLOBAL"; break;
                default:         str_mem = "UNKNOWN"; break;
            }
        } else  {
            str_mem = "SYMBOL";
        }

        printf("%s, TYPE_%s, CLS_%s, MEM_%s\n", (*s)->name, str_type, str_cls, str_mem);
    }
}
