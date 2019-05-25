#include "type_analysis.h"

void cast(type_t *dst, type_t *src)
{
    if (src->num_elem > -1) {
        if (dst->num_elem > -1) {
            if (src->type_base != dst->type_base) {
                tkerr(current_token, "an array cannot be converted to an array of another type");
            }
        } else {
            tkerr(current_token, "an array cannot be converted to a non-array");
        }
    }
    else {
        if (dst->num_elem > -1) {
            tkerr(current_token, "a non-array cannot be converted to an array");
        }
    }

    switch (src->type_base) {
        case TB_CHAR:
        case TB_INT:
        case TB_DOUBLE:
            switch (dst->type_base) {
                case TB_CHAR:
                case TB_INT:
                case TB_DOUBLE:
                    return;
            }
            break;
        case TB_STRUCT:
            if (dst->type_base == TB_STRUCT) {
                if (src->s != dst->s) {
                    tkerr(current_token, "a structure cannot be converted to another one");
                }
                return;
            }
    }

    tkerr(current_token, "incompatible types");
}

type_t get_arith_type(type_t *s1, type_t *s2)
{
    switch (s1->type_base) {
        case TB_DOUBLE:
            switch (s2->type_base) {
                case TB_DOUBLE:
                case TB_INT:
                case TB_CHAR:
                    return *s1;
            }
            break;

        case TB_INT:
            switch (s2->type_base) {
                case TB_INT:
                case TB_CHAR:
                    return *s1;
                case TB_DOUBLE:
                    return *s2;
            }
            break;

        case TB_CHAR:
            switch (s2->type_base) {
                case TB_CHAR:
                    return *s1;
                case TB_INT:
                case TB_DOUBLE:
                    return *s2;
            }
            break;
    }
    
    return *s1; // TODO
}
