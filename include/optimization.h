#ifndef _H_OPTIMIZATION
#define _H_OPTIMIZATION

#include "common.h"
#include "virtual_machine.h"

int need_target_instr(instr_t *crt);
int is_target(instr_t *crt);

void move_target(instr_t *src, instr_t *dst);
void delete_instr(instr_t *i);

unsigned int pass_offset();
unsigned int pass_del_duplications();
unsigned int pass_del_nops();
unsigned int pass_pushfp_int();

void optimize();

#endif
