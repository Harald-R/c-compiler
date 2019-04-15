#ifndef _H_SYMBOL_TABLE
#define _H_SYMBOL_TABLE

#include "common.h"

enum {
    TB_INT,
    TB_DOUBLE,
    TB_CHAR,
    TB_STRUCT,
    TB_VOID
};

enum {
    CLS_VAR,
    CLS_FUNC,
    CLS_EXTFUNC,
    CLS_STRUCT
};

enum {
    MEM_GLOBAL,
    MEM_ARG,
    MEM_LOCAL
};

struct _symbol;
typedef struct _symbol symbol_t;

// symbols_t - an abstraction of a dynamic array of symbols
typedef struct {
    symbol_t **begin; // the beginning of the symbols
    symbol_t **end;   // the position after the last symbol
    symbol_t **after; // the position after the allocated space
} symbols_t;

// type_t - base type of the symbol
typedef struct {
    int type_base; // TB_*
    symbol_t *s;   // struct definition for TB_STRUCT
    int num_elem;  // > 0 for array of given size,
                   // = 0 for array without size,
                   // < 0 for non-array
} type_t;

// symbol_t - the main structure of a symbol
struct _symbol {
    const char *name; // a reference to the name stored in a token
    type_t type;
    int cls;          // CLS_*
    int mem;          // MEM_*
    int depth;        // 0  - global
                      // 1  - in function
                      // 2+ - nested blocks in function
    union {
        symbols_t args;    // used only for functions
        symbols_t members; // used only for structs
    };
};

extern symbols_t symbols;
extern int current_depth;
extern symbol_t *current_struct;
extern symbol_t *current_func;

void init_symbol_table(symbols_t *symbols);

void init_symbols(symbols_t *symbols);
symbol_t *add_symbol(symbols_t *symbols, const char *name, int cls);
symbol_t *find_symbol(symbols_t *symbols, const char *name);

void add_ext_funcs(symbols_t *symbols);
symbol_t *add_ext_func(symbols_t *symbols, const char *name, type_t type);
symbol_t *add_func_arg(symbol_t *func, const char *name, type_t type);

void add_var(token_t *token_name, type_t *type);
void delete_symbols_after(symbols_t *symbols, symbol_t *start);

void print_symbol_table();

type_t create_type(int type_base, int num_elem);

#endif
