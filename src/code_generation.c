#include "code_generation.h"

int type_base_size(type_t *type)
{
    int size = 0;
    symbol_t **is;

    switch (type->type_base) {
        case TB_VOID:   size = 0; break;
        case TB_INT:    size = sizeof(long int); break;
        case TB_DOUBLE: size = sizeof(double); break;
        case TB_CHAR:   size = sizeof(char); break;
        case TB_STRUCT:
            for (is = type->s->members.begin; is != type->s->members.end; ++is) {
                size += type_full_size(&(*is)->type);
            }
            break;
        default: err("invalid type_base: %d", type->type_base);
    }
    return size;
}

int type_full_size(type_t *type)
{
    return type_base_size(type) * (type->num_elem > 0 ? type->num_elem : 1);
}

int type_arg_size(type_t *type)
{
    if (type->num_elem >= 0) return sizeof(void*);
    return type_base_size(type);
}

