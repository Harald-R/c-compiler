#include "optimization.h"

int need_target_instr(instr_t *crt)
{
    switch(crt->opcode) {
        case O_CALL:
        case O_JF_A: case O_JF_C: case O_JF_D: case O_JF_I:
        case O_JMP:
        case O_JT_A: case O_JT_C: case O_JT_D: case O_JT_I:
            return 1;
        default:
            return 0;
    }
}

int is_target(instr_t *crt)
{
    instr_t *i;
    for (i = instructions; i; i = i->next) {
        if (need_target_instr(i) && i->args[0].addr == crt)
            return 1;
    }
    return 0;
}

void move_target(instr_t *src, instr_t *dst)
{
    instr_t *i;
    for (i = instructions; i; i = i->next) {
        if (need_target_instr(i) && i->args[0].addr == src)
            i->args[0].addr = dst;
    }
}

void delete_instr(instr_t *i)
{
    instr_t *prev = i->prev;
    instr_t *next = i->next;

    if (prev == NULL)
        instructions = next;
    else
        prev->next = next;

    if (next == NULL)
        last_instruction = prev;
    else
        next->prev = prev;

    free(i);
}

// PUSHCT_I sizeof(long int); MUL_I; OFFSET -> OFFSET_I
unsigned int pass_offset()
{
    instr_t *i1, *i2, *i3;
    unsigned int optimized = 0;

    for (i1 = instructions;; i1 = i2) {
        i2 = i1->next;
        if (!i2) break;
        if (i1->opcode != O_PUSHCT_I || i1->args[0].i != sizeof(long int)) continue;
        if (i2->opcode != O_MUL_I) continue;
        if (is_target(i2)) continue;
        i3 = i2->next;
        if (!i3 || i3->opcode != O_OFFSET) continue;
        if (is_target(i3)) continue;
        i3->opcode = O_OFFSET_I;
        move_target(i1, i3);
        delete_instr(i1);
        delete_instr(i2);
        i2 = i3;
        optimized = 1;
    }

    DEBUG_PRINTF("pass_offset: %d\n", optimized);
    return optimized;
}

// INSERT sizeof(void*)+n,n; STORE n; DROP n -> STORE n
unsigned int pass_del_duplications()
{
    instr_t *i1, *i2, *i3;
    int n;
    unsigned int optimized = 0;

    for (i1 = instructions;; i1 = i2) {
        i2 = i1->next;
        if (!i2) break;
        if (i1->opcode != O_INSERT) continue;
        n = i1->args[1].i;
        if (i1->args[0].i != (long int)sizeof(void*) + n) continue;
        if (i2->opcode != O_STORE || i2->args[0].i != n) continue;
        if (is_target(i2)) continue;
        i3 = i2->next;
        if (!i3 || i3->opcode != O_DROP || i3->args[0].i != n) continue;
        if (is_target(i3)) continue;
        move_target(i1, i2);
        delete_instr(i1);
        delete_instr(i3);
        optimized = 1;
    }

    DEBUG_PRINTF("pass_del_duplications: %d\n", optimized);
    return optimized;
}

// Delete NOP instructions
unsigned int pass_del_nops()
{
    instr_t *i1, *i2;
    unsigned int optimized = 0;

    for (i1 = instructions; i1; i1 = i2) {
        i2 = i1->next;
        if (i1->opcode != O_NOP) continue;
        if (is_target(i1)) continue;
        delete_instr(i1);
        optimized = 1;
    }

    DEBUG_PRINTF("pass_del_nops: %d\n", optimized);
    return optimized;
}

// PUSHFPADDR i; LOAD sizeof(long int) -> PUSHFP_I i
unsigned int pass_pushfp_int()
{
    instr_t *i1, *i2;
    unsigned int optimized = 0;

    for (i1 = instructions;; i1 = i2) {
        i2 = i1->next;
        if (!i2) break;
        if (i1->opcode != O_PUSHFPADDR) continue;
        if (i2->opcode != O_LOAD || i2->args[0].i != sizeof(long int)) continue;
        if (is_target(i2)) continue;
        i2->opcode = O_PUSHFP_I;
        i2->args[0].i = i1->args[0].i;
        move_target(i1, i2);
        delete_instr(i1);
        optimized = 1;
    }

    DEBUG_PRINTF("pass_pushfp_int: %d\n", optimized);
    return optimized;
}

void optimize()
{
    unsigned int optimized;

    DEBUG_PRINTF("\nStarting optimizations...\n");
    do {
        optimized = 0;

        optimized |= pass_offset();
        optimized |= pass_del_duplications();
        optimized |= pass_del_nops();
        //optimized |= pass_pushfp_int();
    } while(optimized);
}

