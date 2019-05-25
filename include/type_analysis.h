#ifndef _H_TYPE_ANALYSIS
#define _H_TYPE_ANALYSIS

#include "common.h"
#include "symbol_table.h"

typedef union {
    long int i;      // int, char
    double d;        // double
    const char *str; // char[]
} ct_val_t;

typedef struct {
    type_t type;     // type of the result
    int is_lval;     // is a l-val
    int is_ct_val;   // is a constant value (int, real, char, char[])
    ct_val_t ct_val; // the constant value
} ret_val_t;

void cast(type_t *dst, type_t *src);
type_t get_arith_type(type_t *s1, type_t *s2);

#endif
