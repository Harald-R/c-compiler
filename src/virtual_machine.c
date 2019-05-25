#include "virtual_machine.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

char stack[STACK_SIZE];
char *SP;          // Stack Pointer
char *stack_after; // first byte after stack

char globals[GLOBAL_SIZE];
int num_globals;
instr_t *instructions, *last_instruction;

void run(instr_t *IP)
{
    long int i1, i2;
    double d1, d2;
    char c1, c2;
    char *a1, *a2;
    char *FP = 0, *oldSP;

    SP = stack;
    stack_after = stack + STACK_SIZE;

    while (1) {
        printf("%p/%ld\t", IP, SP-stack);

        switch (IP->opcode) {
            case O_ADD_C:
                c2 = popc();
                c1 = popc();
                printf("ADD_C\t(%c+%c -> %c)\n", c1, c2, c1+c2);
                pushd(c1+c2);
                IP = IP->next;
                break;

            case O_ADD_D:
                d2 = popd();
                d1 = popd();
                printf("ADD_D\t(%g+%g -> %g)\n", d1, d2, d1+d2);
                pushd(d1+d2);
                IP = IP->next;
                break;

            case O_ADD_I:
                i2 = popi();
                i1 = popi();
                printf("ADD_I\t(%li+%li -> %li)\n", i1, i2, i1+i2);
                pushi(i1+i2);
                IP = IP->next;
                break;

            case O_AND_A:
                a2 = popa();
                a1 = popa();
                printf("AND_A\t(%p AND %p -> %d)\n", a1, a2, a1&&a2);
                pushi(a1&&a2);
                IP = IP->next;
                break;

            case O_AND_C:
                c2 = popc();
                c1 = popc();
                printf("AND_C\t(%c AND %c -> %d)\n", c1, c2, c1&&c2);
                pushi(c1&&c2);
                IP = IP->next;
                break;

            case O_AND_D:
                d2 = popd();
                d1 = popd();
                printf("AND_D\t(%g AND %g -> %d)\n", d1, d2, d1&&d2);
                pushi(d1&&d2);
                IP = IP->next;
                break;

            case O_AND_I:
                i2 = popi();
                i1 = popi();
                printf("AND_I\t(%ld AND %ld -> %d)\n", i1, i2, i1&&i2);
                pushi(i1&&i2);
                IP = IP->next;
                break;

            case O_CALL:
                a1 = IP->args[0].addr;
                printf("CALL\t%p\n", a1);
                pusha(IP->next);
                IP = (instr_t *)a1;
                break;

            case O_CALLEXT:
                printf("CALLEXT\t%p\n", IP->args[0].addr);
                (*(void(*)())IP->args[0].addr)();
                IP = IP->next;
                break;

            case O_CAST_C_D:
                c1 = popc();
                d1 = (double)c1;
                printf("CAST_C_D\t(%c -> %g)\n", c1, d1);
                pushd(d1);
                IP = IP->next;
                break;

            case O_CAST_C_I:
                c1 = popc();
                i1 = (long int)c1;
                printf("CAST_C_I\t(%c -> %ld)\n", c1, i1);
                pushi(i1);
                IP = IP->next;
                break;

            case O_CAST_D_C:
                d1 = popd();
                c1 = (char)d1;
                printf("CAST_D_C\t(%g -> %c)\n", d1, c1);
                pushc(c1);
                IP = IP->next;
                break;

            case O_CAST_D_I:
                d1 = popd();
                i1 = (long int)d1;
                printf("CAST_D_I\t(%g -> %ld)\n", d1, i1);
                pushi(i1);
                IP = IP->next;
                break;

            case O_CAST_I_C:
                i1 = popi();
                c1 = (char)c1;
                printf("CAST_I_C\t(%ld -> %c)\n", i1, c1);
                pushc(c1);
                IP = IP->next;
                break;

            case O_CAST_I_D:
                i1 = popi();
                d1 = (double)i1;
                printf("CAST_I_D\t(%ld -> %g)\n", i1, d1);
                pushd(d1);
                IP = IP->next;
                break;

            case O_DIV_C:
                c2 = popc();
                c1 = popc();
                printf("DIV_C\t(%c/%c -> %c)\n", c1, c2, c1/c2);
                pushc(c1/c2);
                IP = IP->next;
                break;

            case O_DIV_D:
                d2 = popd();
                d1 = popd();
                printf("DIV_D\t(%g/%g -> %g)\n", d1, d2, d1/d2);
                pushd(d1/d2);
                IP = IP->next;
                break;

            case O_DIV_I:
                i2 = popi();
                i1 = popi();
                printf("DIV_I\t(%ld/%ld -> %ld)\n", i1, i2, i1/i2);
                pushi(i1/i2);
                IP = IP->next;
                break;

            case O_DROP:
                i1 = IP->args[0].i;
                printf("DROP\t%ld\n", i1);
                if (SP-i1 < stack)
                    err("not enough stack bytes");
                SP -= i1;
                IP = IP->next;
                break;

            case O_ENTER:
                i1 = IP->args[0].i;
                printf("ENTER\t%ld\n", i1);
                pusha(FP);
                FP = SP;
                SP += i1;
                IP = IP->next;
                break;

            case O_EQ_A:
                a2 = popa();
                a1 = popa();
                printf("EQ_A\t(%p==%p -> %d)\n", a1, a2, a1==a2);
                pushi(a1==a2);
                IP = IP->next;
                break;

            case O_EQ_C:
                c2 = popc();
                c1 = popc();
                printf("EQ_C\t(%c==%c -> %d)\n", c1, c2, c1==c2);
                pushi(c1==c2);
                IP = IP->next;
                break;

            case O_EQ_D:
                d2 = popd();
                d1 = popd();
                printf("EQ_D\t(%g==%g -> %d)\n", d1, d2, d1==d2);
                pushi(d1==d2);
                IP = IP->next;
                break;

            case O_EQ_I:
                i2 = popi();
                i1 = popi();
                printf("EQ_I\t(%ld==%ld -> %d)\n", i1, i2, i1==i2);
                pushi(i1==i2);
                IP = IP->next;
                break;

            case O_GREATER_C:
                c2 = popc();
                c1 = popc();
                printf("GREATER_C\t(%c>%c -> %d)\n", c1, c2, c1>c2);
                pushi(c1>c2);
                IP = IP->next;
                break;

            case O_GREATER_D:
                d2 = popd();
                d1 = popd();
                printf("GREATER_D\t(%g>%g -> %d)\n", d1, d2, d1>d2);
                pushi(d1>d2);
                IP = IP->next;
                break;

            case O_GREATER_I:
                i2 = popi();
                i1 = popi();
                printf("GREATER_I\t(%ld>%ld -> %d)\n", i1, i2, i1>i2);
                pushi(i1>i2);
                IP = IP->next;
                break;

            case O_GREATEREQ_C:
                c2 = popc();
                c1 = popc();
                printf("GREATEREQ_C\t(%c>=%c -> %d)\n", c1, c2, c1>=c2);
                pushi(c1>=c2);
                IP = IP->next;
                break;

            case O_GREATEREQ_D:
                d2 = popd();
                d1 = popd();
                printf("GREATEREQ_D\t(%g>=%g -> %d)\n", d1, d2, d1>=d2);
                pushi(d1>=d2);
                IP = IP->next;
                break;

            case O_GREATEREQ_I:
                i2 = popi();
                i1 = popi();
                printf("GREATEREQ_I\t(%ld>=%ld -> %d)\n", i1, i2, i1>=i2);
                pushi(i1>=i2);
                IP = IP->next;
                break;

            case O_HALT:
                printf("HALT\n");
                return;

            case O_INSERT:
                i1 = IP->args[0].i;           // Destination address
                i2 = IP->args[1].i;           // Number of bytes
                printf("INSERT\t%ld,%ld\n", i1, i2);
                if(SP+i2 > stack_after)
                    err("out of stack");
                memmove(SP-i1+i2, SP-i1, i1); // Make room
                memmove(SP-i1, SP+i2, i2);    // Dup
                SP += i2;
                IP = IP->next;
                break;

            case O_JF_A: // ?
                a1 = popa();
                printf("JF_A\t%p\t(%p)\n", IP->args[0].addr, a1);
                IP = !a1 ? IP->args[0].addr : IP->next;
                break;

            case O_JF_C: // ?
                c1 = popc();
                printf("JF_C\t%p\t(%c)\n", IP->args[0].addr, c1);
                IP = !c1 ? IP->args[0].addr : IP->next;
                break;

            case O_JF_D: // ?
                d1 = popd();
                printf("JF_D\t%p\t(%g)\n", IP->args[0].addr, d1);
                IP = !d1 ? IP->args[0].addr : IP->next;
                break;

            case O_JF_I: // ?
                i1 = popi();
                printf("JF_I\t%p\t(%ld)\n", IP->args[0].addr, i1);
                IP = !i1 ? IP->args[0].addr : IP->next;
                break;

            case O_JMP:
                printf("JMP\t%p\n", IP->args[0].addr);
                IP = IP->args[0].addr;
                break;

            case O_JT_A: // ?
                a1 = popa();
                printf("JT_A\t%p\t(%p)\n", IP->args[0].addr, a1);
                IP = a1 ? IP->args[0].addr : IP->next;
                break;

            case O_JT_C: // ?
                c1 = popc();
                printf("JT_C\t%p\t(%c)\n", IP->args[0].addr, c1);
                IP = c1 ? IP->args[0].addr : IP->next;
                break;

            case O_JT_D: // ?
                d1 = popd();
                printf("JT_D\t%p\t(%g)\n", IP->args[0].addr, d1);
                IP = d1 ? IP->args[0].addr : IP->next;
                break;

            case O_JT_I:
                i1 = popi();
                printf("JT\t%p\t(%ld)\n", IP->args[0].addr, i1);
                IP = i1 ? IP->args[0].addr : IP->next;
                break;
            
            case O_LESS_C:
                c2 = popc();
                c1 = popc();
                printf("LESS_C\t(%c<%c -> %d)\n", c1, c2, c1<c2);
                pushi(c1<c2);
                IP = IP->next;
                break;

            case O_LESS_D:
                d2 = popd();
                d1 = popd();
                printf("LESS_D\t(%g<%g -> %d)\n", d1, d2, d1<d2);
                pushi(d1<d2);
                IP = IP->next;
                break;

            case O_LESS_I:
                i2 = popi();
                i1 = popi();
                printf("LESS_I\t(%ld<%ld -> %d)\n", i1, i2, i1<i2);
                pushi(i1<i2);
                IP = IP->next;
                break;

            case O_LESSEQ_C:
                c2 = popc();
                c1 = popc();
                printf("LESSEQ_C\t(%c<=%c -> %d)\n", c1, c2, c1<=c2);
                pushi(c1<=c2);
                IP = IP->next;
                break;

            case O_LESSEQ_D:
                d2 = popd();
                d1 = popd();
                printf("LESSEQ_D\t(%g<=%g -> %d)\n", d1, d2, d1<=d2);
                pushi(d1<=d2);
                IP = IP->next;
                break;

            case O_LESSEQ_I:
                i2 = popi();
                i1 = popi();
                printf("LESSEQ_I\t(%ld<=%ld -> %d)\n", i1, i2, i1<=i2);
                pushi(i1<=i2);
                IP = IP->next;
                break;

            case O_LOAD:
                i1 = IP->args[0].i;
                a1 = popa();
                printf("LOAD\t%ld\t(%p)\n", i1, a1);
                if(SP+i1 > stack_after)
                    err("out of stack");
                memcpy(SP, a1, i1);
                SP += i1;
                IP = IP->next;
                break;

            case O_MUL_C:
                c2 = popc();
                c1 = popc();
                printf("MUL_C\t(%c/%c -> %c)\n", c1, c2, c1*c2);
                pushc(c1*c2);
                IP = IP->next;
                break;

            case O_MUL_D:
                d2 = popd();
                d1 = popd();
                printf("MUL_D\t(%g/%g -> %g)\n", d1, d2, d1*d2);
                pushd(d1*d2);
                IP = IP->next;
                break;

            case O_MUL_I:
                i2 = popi();
                i1 = popi();
                printf("MUL_I\t(%ld/%ld -> %ld)\n", i1, i2, i1*i2);
                pushi(i1*i2);
                IP = IP->next;
                break;

            case O_NEG_C:
                c1 = popc();
                printf("NEG_C\t(%c -> %c)\n", c1, -c1);
                pushc(-c1);
                IP = IP->next;
                break;

            case O_NEG_D:
                d1 = popd();
                printf("NEG_D\t(%g -> %g)\n", d1, -d1);
                pushd(-d1);
                IP = IP->next;
                break;

            case O_NEG_I:
                i1 = popi();
                printf("NEG_I\t(%ld -> %ld)\n", i1, -i1);
                pushi(-i1);
                IP = IP->next;
                break;

            case O_NOP:
                IP = IP->next; // ?
                break;

            case O_NOT_A:
                a1 = popa();
                printf("NOT_A\t(%p -> %d)\n", a1, !a1);
                pushi(!a1);
                IP = IP->next;
                break;

            case O_NOT_C:
                c1 = popc();
                printf("NOT_C\t(%c -> %d)\n", c1, !c1);
                pushi(!c1);
                IP = IP->next;
                break;

            case O_NOT_D:
                d1 = popd();
                printf("NOT_D\t(%g -> %d)\n", d1, !d1);
                pushi(!d1);
                IP = IP->next;
                break;

            case O_NOT_I:
                i1 = popi();
                printf("NOT_I\t(%ld -> %d)\n", i1, !i1);
                pushi(!i1);
                IP = IP->next;
                break;

            case O_NOTEQ_A:
                a2 = popa();
                a1 = popa();
                printf("NOTEQ_A\t(%p==%p -> %d)\n", a1, a2, a1!=a2);
                pushi(a1!=a2);
                IP = IP->next;
                break;

            case O_NOTEQ_C:
                c2 = popc();
                c1 = popc();
                printf("NOTEQ_C\t(%c==%c -> %d)\n", c1, c2, c1!=c2);
                pushi(c1!=c2);
                IP = IP->next;
                break;

            case O_NOTEQ_D:
                d2 = popd();
                d1 = popd();
                printf("NOTEQ_D\t(%g==%g -> %d)\n", d1, d2, d1!=d2);
                pushi(d1!=d2);
                IP = IP->next;
                break;

            case O_NOTEQ_I:
                i2 = popi();
                i1 = popi();
                printf("NOTEQ_I\t(%ld==%ld -> %d)\n", i1, i2, i1!=i2);
                pushi(i1!=i2);
                IP = IP->next;
                break;

            case O_OFFSET:
                i1 = popi();
                a1 = popa();
                printf("OFFSET\t(%p+%ld -> %p)\n", a1, i1, a1+i1);
                pusha(a1+i1);
                IP = IP->next;
                break;

            case O_OR_A:
                a2 = popa();
                a1 = popa();
                printf("OR_A\t(%p OR %p -> %d)\n", a1, a2, a1||a2);
                pushi(a1||a2);
                IP = IP->next;
                break;

            case O_OR_C:
                c2 = popc();
                c1 = popc();
                printf("OR_C\t(%c OR %c -> %d)\n", c1, c2, c1||c2);
                pushi(c1||c2);
                IP = IP->next;
                break;

            case O_OR_D:
                d2 = popd();
                d1 = popd();
                printf("OR_D\t(%g OR %g -> %d)\n", d1, d2, d1||d2);
                pushi(d1||d2);
                IP = IP->next;
                break;

            case O_OR_I:
                i2 = popi();
                i1 = popi();
                printf("OR_I\t(%ld OR %ld -> %d)\n", i1, i2, i1||i2);
                pushi(i1||i2);
                IP = IP->next;
                break;

            case O_PUSHFPADDR:
                i1 = IP->args[0].i;
                printf("PUSHFPADDR\t%ld\t(%p)\n", i1, FP+i1);
                pusha(FP+i1);
                IP = IP->next;
                break;

            case O_PUSHCT_A:
                a1 = IP->args[0].addr;
                printf("PUSHCT_A\t%p\n", a1);
                pusha(a1);
                IP = IP->next;
                break;

            case O_PUSHCT_C:
                c1 = IP->args[0].i;
                printf("PUSHCT_C\t%c\n", c1);
                pushc(c1);
                IP = IP->next;
                break;

            case O_PUSHCT_D:
                d1 = IP->args[0].d;
                printf("PUSHCT_D\t%f\n", d1);
                pushd(d1);
                IP = IP->next;
                break;

            case O_PUSHCT_I:
                i1 = IP->args[0].i;
                printf("PUSHCT_I\t%ld\n", i1);
                pushi(i1);
                IP = IP->next;
                break;

            case O_RET:
                i1 = IP->args[0].i;
                i2 = IP->args[1].i; // sizeof(retType)
                printf("RET\t%ld,%ld\n", i1, i2);
                oldSP = SP;
                SP = FP;
                FP = popa();
                IP = popa();
                if(SP-i1 < stack)
                    err("not enough stack bytes");
                SP -= i1;
                memmove(SP, oldSP-i2, i2);
                SP += i2;
                break;

            case O_STORE:
                i1 = IP->args[0].i;
                if (SP - (sizeof(void*) + i1) < stack)
                    err("not enough stack bytes for SET");
                a1 = *(void**)(SP - (sizeof(void*) + i1));
                printf("STORE\t%ld\t(%p)\n", i1, a1);
                memcpy(a1, SP-i1, i1);
                SP -= sizeof(void*) + i1;
                IP = IP->next;
                break;

            case O_SUB_C:
                c2 = popc();
                c1 = popc();
                printf("SUB_C\t(%c-%c -> %c)\n", c2, c1, c2-c1);
                pushc(c2-c1);
                IP = IP->next;
                break;

            case O_SUB_D:
                d2 = popd();
                d1 = popd();
                printf("SUB_D\t(%g-%g -> %g)\n", d2, d1, d2-d1);
                pushd(d2-d1);
                IP = IP->next;
                break;

            case O_SUB_I:
                i2 = popi();
                i1 = popi();
                printf("SUB_I\t(%ld-%ld -> %ld)\n", i2, i1, i2-i1);
                pushi(i2-i1);
                IP = IP->next;
                break;

            default:
                err("invalid opcode: %d", IP->opcode);
        }
    }
}

void pusha(void *a)
{
    if (SP + sizeof(void*) > stack_after)
        err("out of stack");

    *(void**)SP = a;
    SP += sizeof(void*);
}

void *popa()
{
    SP -= sizeof(void*);
    if (SP < stack)
        err("not enough stack bytes for popa");

    return *(void**)SP;
}

void pushc(char c)
{
    if (SP + sizeof(char) > stack_after)
        err("out of stack");

    *(char*)SP = c;
    SP += sizeof(char);
}

char popc()
{
    SP -= sizeof(char);
    if (SP < stack)
        err("not enough stack bytes for popc");

    return *(char*)SP;
}

void pushd(double d)
{
    if (SP + sizeof(double) > stack_after)
        err("out of stack");

    *(double*)SP = d;
    SP += sizeof(double);
}

double popd()
{
    SP -= sizeof(double);
    if (SP < stack)
        err("not enough stack bytes for popd");

    return *(double*)SP;
}

void pushi(long int i)
{
    if (SP + sizeof(long int) > stack_after)
        err("out of stack");

    *(long int*)SP = i;
    SP += sizeof(long int);
}

long int popi()
{
    SP -= sizeof(long int);
    if (SP < stack)
        err("not enough stack bytes for popi");

    return *(long int*)SP;
}

void add_cast_instr(instr_t *after, type_t *actual_type, type_t *needed_type)
{
    if (actual_type->num_elem >= 0 || needed_type->num_elem >= 0) return;

    switch (actual_type->type_base) {
        case TB_CHAR:
            switch (needed_type->type_base) {
                case TB_CHAR:   break;
                case TB_INT:    add_instr_after(after, O_CAST_C_I); break;
                case TB_DOUBLE: add_instr_after(after, O_CAST_C_D); break;
            }
            break;
        case TB_INT:
            switch (needed_type->type_base) {
                case TB_CHAR:   add_instr_after(after, O_CAST_I_C); break;
                case TB_INT:    break;
                case TB_DOUBLE: add_instr_after(after, O_CAST_I_D); break;
            }
            break;
        case TB_DOUBLE:
            switch (needed_type->type_base) {
                case TB_CHAR:   add_instr_after(after, O_CAST_D_C); break;
                case TB_INT:    add_instr_after(after, O_CAST_D_I); break;
                case TB_DOUBLE: break;
            }
            break;
    }
}

instr_t *create_instr(int opcode)
{
    instr_t *i;
    SAFEALLOC(i, instr_t)
    i->opcode = opcode;
    return i;
}

instr_t *append_instr(instr_t *i)
{
    if (instructions == NULL) {
        i->prev = NULL;
        i->next = NULL;
        instructions = i;
        last_instruction = instructions;

        return i;
    }

    i->prev = last_instruction;
    i->next = NULL;
    last_instruction->next = i;
    last_instruction = i;

    return i;
}

void insert_instr_after(instr_t *after, instr_t *i)
{
    i->next = after->next;
    i->prev = after;
    after->next = i;
    if (i->next == NULL)
        last_instruction = i;
}

instr_t *add_instr(int opcode)
{
    instr_t *i = create_instr(opcode);
    i->next = NULL;
    i->prev = last_instruction;

    if (last_instruction) {
        last_instruction->next = i;
    } else {
        instructions = i;
    }

    last_instruction = i;
    return i;
}

instr_t *add_instr_after(instr_t *after, int opcode)
{
    instr_t *i = create_instr(opcode);
    insert_instr_after(after, i);
    return i;
}

instr_t *add_instr_A(int opcode, void *addr)
{
    instr_t *i = add_instr(opcode);
    i->args[0].addr = addr;
    return i;
}

instr_t *add_instr_I(int opcode, long int val)
{
    instr_t *i = add_instr(opcode);
    i->args[0].i = val;
    return i;
}

instr_t *add_instr_II(int opcode, long int val1, long int val2)
{
    instr_t *i = add_instr(opcode);
    i->args[0].i = val1;
    i->args[1].i = val2;
    return i;
}

void delete_instructions_after(instr_t *start)
{
    if (start == NULL) return;
    
    instr_t *i = start->next;
    start->next = NULL;

    while(i) {
        instr_t *crt_i = i;
        i = i->next;
        free(crt_i);
    }

    last_instruction = start;
}

void *alloc_global(int size)
{
    void *p = globals + num_globals;
    if (num_globals + size > GLOBAL_SIZE)
        err("insufficient globals space");
    num_globals += size;
    return p;
}

void put_s()
{
    printf("#%s\n", (char*)popa());
}

void put_i()
{
    printf("#%ld\n", popi());
}

void put_d()
{
    printf("#%f\n", popd());
}

void put_c()
{
    printf("#%c\n", popc());
}

void get_s()
{
    // TODO: dynamically allocate memory for string
    char s[100];
    fgets(s, 100, stdin);
    pusha(s);
}

void get_i()
{
    long int i;
    scanf("%ld", &i);
    pushi(i);
}

void get_d()
{
    double d;
    scanf("%lf", &d);
    pushd(d);
}

void get_c()
{
    char c;
    scanf("%c", &c);
    pushc(c);

}

void seconds()
{
    // TODO: push correct? date to stack
    double date = time(NULL);
    pushd(date);
}
