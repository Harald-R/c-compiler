#include "syntax_analyzer.h"
#include "symbol_table.h"
#include "type_analysis.h"
#include "virtual_machine.h"
#include "code_generation.h"

token_t *consumed_token;
instr_t *crt_loop_end;
int offset = 0;
int size_args = 0;

int analyze_syntax()
{
    return unit();
}

int consume(int code)
{
    DEBUG_PRINTF("consume(%s)? => ", id_to_str(code));
    if (current_token->code == code) {
        consumed_token = current_token;
        current_token = current_token->next;
        DEBUG_PRINTF("OK\n");
        return 1;
    }
    DEBUG_PRINTF("not OK (%s)\n", id_to_str(current_token->code));
    return 0;
}

int unit()
{
    instr_t *label_main = add_instr(O_CALL);
    add_instr(O_HALT);

    while (declStruct() || declFunc() || declVar());
    label_main->args[0].addr = require_symbol(&symbols, "main")->addr;

    if (consume(END)) {
        return 1;
    }
    else tkerr(current_token, "invalid expression");

    return 0;
}

instr_t *get_rval(ret_val_t *rv)
{
    if (rv->is_lval) {
        switch (rv->type.type_base) {
            case TB_INT:
            case TB_DOUBLE:
            case TB_CHAR:
            case TB_STRUCT:
                add_instr_I(O_LOAD, type_arg_size(&rv->type));
                break;
            default: tkerr(current_token, "unhandled type: %d", rv->type.type_base);
        }
    }
    return last_instruction;
}

instr_t *create_cond_jump(ret_val_t *rv)
{
    if (rv->type.num_elem >= 0) { // arrays
        return add_instr(O_JF_A);
    } else {                      // non-arrays
        get_rval(rv);
        switch (rv->type.type_base) {
            case TB_CHAR:   return add_instr(O_JF_C);
            case TB_DOUBLE: return add_instr(O_JF_D);
            case TB_INT:    return add_instr(O_JF_I);
            default: return NULL;
        }
    }
}

int declStruct()
{
    token_t *start_token = current_token;
    token_t *symbol_token;
    instr_t *old_last_instr = last_instruction;

    if (consume(STRUCT)) {
        if (consume(ID)) {
            symbol_token = consumed_token;

            if (consume(LACC)) {
                offset = 0;

                // Add struct symbol to the symbol table
                if (find_symbol(&symbols, symbol_token->text)) {
                    tkerr(current_token, "symbol redefinition: %s", symbol_token->text);
               }
                current_struct = add_symbol(&symbols, symbol_token->text, CLS_STRUCT);
                init_symbols(&current_struct->members);

                while (declVar());

                if (consume(RACC)) {
                    if (consume(SEMICOLON)) {
                        current_struct = NULL;
                        return 1;
                    }
                    else tkerr(current_token, "missing semicolon");
                }
                else tkerr(current_token, "missing }");
            }
        }
        else tkerr(current_token, "missing id after struct");
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int declFunc()
{
    token_t *start_token = current_token;
    token_t *symbol_token;
    type_t t;
    symbol_t **ps;
    int is_decl_func = 0;
    int has_void_return_type = 0;
    instr_t *old_last_instr = last_instruction;

    if (typeBase(&t)) {
        if (consume(MUL)) {
            t.num_elem = 0;
        } else {
            t.num_elem = -1;
        }

        is_decl_func = 1;

    } else if (consume(VOID)) {
        is_decl_func = 1;
        has_void_return_type = 1;

        t.type_base = TB_VOID;
    }

    if (is_decl_func) {
        if (consume(ID)) {
            symbol_token = consumed_token;
            size_args = offset = 0;

            if (consume(LPAR)) {
                // Add function name in the symbol table 
                if (find_symbol(&symbols, symbol_token->text)) {
                    tkerr(current_token, "symbol redefinition: %s", symbol_token->text);
                }
                current_func = add_symbol(&symbols, symbol_token->text, CLS_FUNC);
                init_symbols(&current_func->args);
                current_func->type = t;
                ++current_depth;

                if (funcArg()) {
                    while (1) {
                        if (consume(COMMA)) {
                            if (funcArg()) {}
                            else tkerr(current_token, "missing argument after comma");
                        }
                        else break;
                    }
                }

                if (consume(RPAR)) {
                    --current_depth;

                    current_func->addr = add_instr(O_ENTER);
                    size_args = offset;
                    // Update args offsets for correct FP indexing
                    for (ps = symbols.begin; ps != symbols.end; ++ps) {
                        if ((*ps)->mem == MEM_ARG) {
                            // 2*sizeof(void*) == sizeof(retAddr) + sizeof(FP)
                            (*ps)->offset -= size_args + 2*sizeof(void*);
                        }
                    }
                    offset = 0;

                    if (stmCompound()) {
                        ((instr_t *)current_func->addr)->args[0].i = offset; // setup the ENTER argument
                        if (current_func->type.type_base == TB_VOID) {
                            add_instr_II(O_RET, size_args, 0);
                        }

                        current_func = NULL;

                        return 1;
                    }
                    else tkerr(current_token, "missing function statement");
                }
                else tkerr(current_token, "expected )");
            }
        }
        else if (has_void_return_type) {
            tkerr(current_token, "expected identifier");
        }
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int declVar()
{
    token_t *start_token = current_token;
    token_t *symbol_token;
    type_t t;
    int is_decl_var = 0;
    instr_t *old_last_instr = last_instruction;

    if (typeBase(&t)) {
        if (consume(ID)) {
            symbol_token = consumed_token;

            if (arrayDecl(&t)) {
                is_decl_var = 1;
            } else {
                t.num_elem = -1;
            }

            add_var(symbol_token, &t);

            while (1) {
                if (consume(COMMA)) {
                    is_decl_var = 1;
                    if (consume(ID)) {
                        symbol_token = consumed_token;

                        if (arrayDecl(&t)) {
                        } else {
                            t.num_elem = -1;
                        }

                        add_var(symbol_token, &t);
                    }
                    else tkerr(current_token, "missing variable name after ,");
                } else {
                    break;
                }
            }

            if (consume(SEMICOLON)) {
                return 1;
            } else if (is_decl_var) {
                tkerr(current_token, "missing semicolon");
            }
        }
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int typeBase(type_t *ret)
{
    token_t *start_token = current_token;
    instr_t *old_last_instr = last_instruction;

    if (consume(INT)) {
        ret->type_base = TB_INT;
        return 1;
    } else if (consume(DOUBLE)) {
        ret->type_base = TB_DOUBLE;
        return 1;
    } else if (consume(CHAR)) {
        ret->type_base = TB_CHAR;
        return 1;
    } else if (consume(STRUCT)) {
        if (consume(ID)) {
            token_t *symbol_token = consumed_token;
            
            symbol_t *s = find_symbol(&symbols, symbol_token->text);
            if (s == NULL)
                tkerr(current_token, "undefined symbol: %s", symbol_token->text);
            if (s->cls != CLS_STRUCT)
                tkerr(current_token, "%s is not a struct", symbol_token->text);

            ret->type_base = TB_STRUCT;
            ret->s = s;

            return 1;
        }
        else tkerr(current_token, "missing id after struct");
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int typeName(type_t *ret)
{
    token_t *start_token = current_token;
    instr_t *old_last_instr = last_instruction;

    if (typeBase(ret)) {
        if (arrayDecl(ret)) { }
        else {
            ret->num_elem = -1;
        }

        return 1;
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;

}

int arrayDecl(type_t *ret)
{
    token_t *start_token = current_token;
    ret_val_t rv;
    instr_t *instr_before_expr;
    instr_t *old_last_instr = last_instruction;

    if (consume(LBRACKET)) {
        ret->num_elem = 0;
        instr_before_expr = last_instruction;

        if (expr(&rv)) {
            if (!rv.is_ct_val)
                tkerr(current_token, "the array size is not a constant");
            if (rv.type.type_base != TB_INT)
                tkerr(current_token, "the array size is not an integer");

            ret->num_elem = rv.ct_val.i;
            delete_instructions_after(instr_before_expr);
        }

        if (consume(RBRACKET)) {
            return 1;
        }
        else tkerr(current_token, "missing ]");
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int funcArg()
{
    token_t *start_token = current_token;
    token_t *symbol_token;
    type_t t;
    instr_t *old_last_instr = last_instruction;

    if (typeBase(&t)) {
        if (consume(ID)) {
            symbol_token = consumed_token;

            if (arrayDecl(&t)) { }
            else {
                t.num_elem = -1;
            }

            symbol_t *s = add_symbol(&symbols, symbol_token->text, CLS_VAR);
            s->mem = MEM_ARG;
            s->type = t;
            s->offset = offset;
            
            s = add_symbol(&current_func->args, symbol_token->text, CLS_VAR);
            s->mem = MEM_ARG;
            s->type = t;
            s->offset = offset;
            offset += type_arg_size(&s->type);

            return 1;
        }
        else tkerr(current_token, "missing argument identifier");
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int stm()
{
    token_t *start_token = current_token;
    ret_val_t rv;
    instr_t *i, *i1, *i2, *i3, *i4, *is, *ib3, *ibs;
    instr_t *old_last_instr = last_instruction;

    if (stmCompound()) return 1;
    else if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr(&rv)) {
                if (rv.type.type_base == TB_STRUCT)
                    tkerr(current_token, "a structure cannot be logically tested");

                if (consume(RPAR)) {
                    i1 = create_cond_jump(&rv);
                    if (stm()) {
                        if (consume(ELSE)) {
                            i2 = add_instr(O_JMP);
                            if (stm()) { }
                            i1->args[0].addr = i2->next;
                            i1 = i2;
                        }
                        i1->args[0].addr = add_instr(O_NOP);

                        return 1;
                    }
                    else tkerr(current_token, "missing if statement");
                }
                else tkerr(current_token, "missing )");
            }
            else tkerr(current_token, "invalid expression after (");
        }
        else tkerr(current_token, "missing ( after if");
    }
    else if (consume(WHILE)) {
        instr_t *old_loop_end = crt_loop_end;
        crt_loop_end = create_instr(O_NOP);
        i1 = last_instruction;

        if (consume(LPAR)) {
            if (expr(&rv)) {
                if (rv.type.type_base == TB_STRUCT)
                    tkerr(current_token, "a structure cannot be logically tested");

                if (consume(RPAR)) {
                    i2 = create_cond_jump(&rv);
                    if (stm()) {
                        add_instr_A(O_JMP, i1->next);
                        append_instr(crt_loop_end);
                        i2->args[0].addr = crt_loop_end;
                        crt_loop_end = old_loop_end;

                        return 1;
                    }
                    else tkerr(current_token, "missing while statement");
                }
                else tkerr(current_token, "missing )");
            }
            else tkerr(current_token, "invalid expression after (");
        }
        else tkerr(current_token, "missing ( after while");
    }
    else if (consume(FOR)) {
        ret_val_t rv1, rv2, rv3;
        instr_t *old_loop_end = crt_loop_end;
        crt_loop_end = create_instr(O_NOP);

        if (consume(LPAR)) {
            if (expr(&rv1)) {
                if (type_arg_size(&rv1.type)) {
                    add_instr_I(O_DROP, type_arg_size(&rv1.type));
                }
            }
            if (consume(SEMICOLON)) {
                i2 = last_instruction;
                if (expr(&rv2)) {
                    i4 = create_cond_jump(&rv2);
                    if(rv2.type.type_base == TB_STRUCT)
                        tkerr(current_token, "a structure cannot be logically tested");
                } else {
                    i4 = NULL;
                }

                if (consume(SEMICOLON)) {
                    ib3 = last_instruction;
                    if (expr(&rv3)) {
                        if (type_arg_size(&rv3.type)) {
                            add_instr_I(O_DROP, type_arg_size(&rv3.type));
                        }
                    }
                    if (consume(RPAR)) {
                        ibs = last_instruction;
                        if (stm()) {
                            if (ib3 != ibs) {
                                i3 = ib3->next;
                                is = ibs->next;
                                ib3->next = is;
                                is->prev = ib3;
                                last_instruction->next = i3;
                                i3->prev = last_instruction;
                                ibs->next = NULL;
                                last_instruction = ibs;
                            }
                            add_instr_A(O_JMP, i2->next);
                            append_instr(crt_loop_end);
                            if (i4)
                                i4->args[0].addr = crt_loop_end;
                            crt_loop_end = old_loop_end;

                            return 1;
                        }
                        else tkerr(current_token, "missing for statement");
                    }
                    else tkerr(current_token, "missing )");
                }
                else tkerr(current_token, "missing semicolon");
            }
            else tkerr(current_token, "missing semicolon");
        }
        else tkerr(current_token, "missing ( after for");
    }
    else if (consume(BREAK)) {
        if (consume(SEMICOLON)) {
            if (!crt_loop_end)
                tkerr(current_token, "break without for or while");
            add_instr_A(O_JMP, crt_loop_end);

            return 1;
        }
        else tkerr(current_token, "missing semicolon");
    }
    else if (consume(RETURN)) {
        if (expr(&rv)) {
            i = get_rval(&rv);
            add_cast_instr(i, &rv.type, &current_func->type);

            if(current_func->type.type_base == TB_VOID)
                tkerr(current_token, "a void function cannot return a value");
            cast(&current_func->type, &rv.type);
        }

        if (consume(SEMICOLON)) {
            if (current_func->type.type_base == TB_VOID) {
                add_instr_II(O_RET, size_args, 0);
            } else {
                add_instr_II(O_RET, size_args, type_arg_size(&current_func->type));
            }

            return 1;
        }
        else tkerr(current_token, "missing semicolon");
    }
    else if (expr(&rv)) {
        if (type_arg_size(&rv.type)) {
            add_instr_I(O_DROP, type_arg_size(&rv.type));
        }

        if (consume(SEMICOLON)) {
            return 1;
        }
        else tkerr(current_token, "expected semicolon");
    }
    else if (consume(SEMICOLON)) {
        return 1;
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int stmCompound()
{
    token_t *start_token = current_token;
    symbol_t *start = symbols.end[-1];
    instr_t *old_last_instr = last_instruction;

    if (consume(LACC)) {
        ++current_depth;
        while (declVar() || stm());

        if (consume(RACC)) {
            --current_depth;
            delete_symbols_after(&symbols, start);

            return 1;
        }
        else tkerr(current_token, "expected }");
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int expr(ret_val_t *rv)
{
    token_t *start_token = current_token;
    instr_t *old_last_instr = last_instruction;

    if (exprAssign(rv)) {
        return 1;
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int exprAssign(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t rve;
    instr_t *i, *old_last_instr = last_instruction;

    if (exprUnary(rv)) {
        if (consume(ASSIGN)) {
            if (exprAssign(&rve)) {
                if (!rv->is_lval)
                    tkerr(current_token, "cannot assign to a non-lval");
                if (rv->type.num_elem > -1 || rve.type.num_elem > -1)
                    tkerr(current_token, "the arrays cannot be assigned");

                cast(&rv->type, &rve.type);

                i = get_rval(&rve);
                add_cast_instr(i, &rve.type, &rv->type);

                // Duplicate the value on top before the destination address
                add_instr_II(O_INSERT,
                        sizeof(void*) + type_arg_size(&rv->type),
                        type_arg_size(&rv->type));
                add_instr_I(O_STORE, type_arg_size(&rv->type)); 
                
                rv->is_ct_val = rv->is_lval = 0;

                return 1;
            }
            else tkerr(current_token, "missing value after = operator");
        }
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    if (exprOr(rv)) {
        return 1;
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int exprOr2(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t rve;
    instr_t *i1, *i2;
    type_t t, t1, t2;
    instr_t *old_last_instr = last_instruction;

    if (consume(OR)) {
        i1 = rv->type.num_elem < 0 ? get_rval(rv) : last_instruction;
        t1 = rv->type;

        if (exprAnd(&rve)) {
            if (rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)
                    tkerr(current_token, "a structure cannot be logically tested");
            if (rv->type.num_elem >= 0) {
                add_instr(O_OR_A);
            } else {
                i2 = get_rval(&rve);
                t2 = rve.type;
                t = get_arith_type(&t1, &t2);
                add_cast_instr(i1, &t1, &t);
                add_cast_instr(i2, &t2, &t);
                switch (t.type_base) {
                    case TB_INT:    add_instr(O_OR_I); break;
                    case TB_DOUBLE: add_instr(O_OR_D); break;
                    case TB_CHAR:   add_instr(O_OR_C); break;
                }
            }

            rv->type = create_type(TB_INT, -1);
            rv->is_ct_val = rv->is_lval = 0;

            if (exprOr2(rv)) {
                return 1;
            }
        }
        else tkerr(current_token, "missing expression after ||");
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 1;
}

int exprOr(ret_val_t *rv)
{
    token_t *start_token = current_token;
    instr_t *old_last_instr = last_instruction;

    if (exprAnd(rv)) {
        if (exprOr2(rv)) {
            return 1;
        }
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int exprAnd2(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t rve;
    instr_t *i1, *i2;
    type_t t, t1, t2;
    instr_t *old_last_instr = last_instruction;

    if (consume(AND)) {
        i1 = rv->type.num_elem < 0 ? get_rval(rv) : last_instruction;
        t1 = rv->type;

        if (exprEq(&rve)) {
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)
                tkerr(current_token, "a structure cannot be logically tested");
            if (rv->type.num_elem >= 0) {
                add_instr(O_AND_A);
            } else {
                i2 = get_rval(&rve);
                t2 = rve.type;
                t = get_arith_type(&t1, &t2);
                add_cast_instr(i1, &t1, &t);
                add_cast_instr(i2, &t2, &t);
                switch (t.type_base) {
                    case TB_INT:    add_instr(O_AND_I); break;
                    case TB_DOUBLE: add_instr(O_AND_D); break;
                    case TB_CHAR:   add_instr(O_AND_C); break;
                }
            }

            rv->type = create_type(TB_INT, -1);
            rv->is_ct_val = rv->is_lval = 0;

            if (exprAnd2(rv)) {
                return 1;
            }
        }
        else tkerr(current_token, "missing expression after &&");
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 1;
}

int exprAnd(ret_val_t *rv)
{
    token_t *start_token = current_token;
    instr_t *old_last_instr = last_instruction;

    if (exprEq(rv)) {
        if (exprAnd2(rv)) {
            return 1;
        }
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int exprEq2(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t rve;
    instr_t *i1, *i2;
    type_t t, t1, t2;
    instr_t *old_last_instr = last_instruction;

    if (consume(EQUAL) || consume(NOTEQ)) {
        token_t *tkop = consumed_token;
        i1 = rv->type.num_elem < 0 ? get_rval(rv) : last_instruction;
        t1 = rv->type;

        if (exprRel(&rve)) {
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)
                tkerr(current_token,"a structure cannot be compared");
            if (rv->type.num_elem >= 0) {
                add_instr(tkop->code == EQUAL ? O_EQ_A : O_NOTEQ_A);
            } else {
                i2 = get_rval(&rve);
                t2 = rve.type;
                t = get_arith_type(&t1, &t2);
                add_cast_instr(i1, &t1, &t);
                add_cast_instr(i2, &t2, &t);
                if (tkop->code == EQUAL) {
                    switch (t.type_base) {
                        case TB_INT:    add_instr(O_EQ_I); break;
                        case TB_DOUBLE: add_instr(O_EQ_D); break;
                        case TB_CHAR:   add_instr(O_EQ_C); break;
                    }
                } else {
                    switch (t.type_base) {
                        case TB_INT:    add_instr(O_NOTEQ_I); break;
                        case TB_DOUBLE: add_instr(O_NOTEQ_D); break;
                        case TB_CHAR:   add_instr(O_NOTEQ_C); break;
                    }

                }
            }

            rv->type = create_type(TB_INT, -1);
            rv->is_ct_val = rv->is_lval = 0;

            if (exprEq2(rv)) {
                return 1;
            }
        }
        else tkerr(current_token, "missing expression after equality comparison");
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 1;
}

int exprEq(ret_val_t *rv)
{
    token_t *start_token = current_token;
    instr_t *old_last_instr = last_instruction;

    if (exprRel(rv)) {
        if (exprEq2(rv)) {
            return 1;
        }
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int exprRel2(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t rve;
    instr_t *i1, *i2;
    type_t t, t1, t2;
    instr_t *old_last_instr = last_instruction;

    if (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)) {
        token_t *tkop = consumed_token;
        i1 = get_rval(rv);
        t1 = rv->type;

        if (exprAdd(&rve)) {
            if(rv->type.num_elem > -1 || rve.type.num_elem > -1)
                tkerr(current_token,"an array cannot be compared");
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)
                tkerr(current_token,"a structure cannot be compared");

            i2 = get_rval(&rve);
            t2 = rve.type;
            t = get_arith_type(&t1, &t2);
            add_cast_instr(i1, &t1, &t);
            add_cast_instr(i2, &t2, &t);
            switch (tkop->code) {
                case LESS:
                    switch (t.type_base) {
                        case TB_INT:    add_instr(O_LESS_I); break;
                        case TB_DOUBLE: add_instr(O_LESS_D); break;
                        case TB_CHAR:   add_instr(O_LESS_C); break;
                    }
                    break;
                case LESSEQ:
                    switch (t.type_base) {
                        case TB_INT:    add_instr(O_LESSEQ_I); break;
                        case TB_DOUBLE: add_instr(O_LESSEQ_D); break;
                        case TB_CHAR:   add_instr(O_LESSEQ_C); break;
                    }
                    break;
                case GREATER:
                    switch (t.type_base) {
                        case TB_INT:    add_instr(O_GREATER_I); break;
                        case TB_DOUBLE: add_instr(O_GREATER_D); break;
                        case TB_CHAR:   add_instr(O_GREATER_C); break;
                    }
                    break;
                case GREATEREQ:
                    switch (t.type_base) {
                        case TB_INT:    add_instr(O_GREATEREQ_I); break;
                        case TB_DOUBLE: add_instr(O_GREATEREQ_D); break;
                        case TB_CHAR:   add_instr(O_GREATEREQ_C); break;
                    }
                    break;
            }

            rv->type = create_type(TB_INT, -1);
            rv->is_ct_val = rv->is_lval = 0;

            if (exprRel2(rv)) {
                return 1;
            }
        }
        else tkerr(current_token, "missing expression after comparison");
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 1;
}

int exprRel(ret_val_t *rv)
{
    token_t *start_token = current_token;
    instr_t *old_last_instr = last_instruction;

    if (exprAdd(rv)) {
        if (exprRel2(rv)) {
            return 1;
        }
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int exprAdd2(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t rve;
    instr_t *i1, *i2;
    type_t t, t1, t2;
    instr_t *old_last_instr = last_instruction;

    if (consume(ADD) || consume(SUB)) {
        token_t *tkop = consumed_token;
        i1 = get_rval(rv);
        t1 = rv->type;

        if (exprMul(&rve)) {
            if(rv->type.num_elem > -1 || rve.type.num_elem > -1)
                tkerr(current_token,"an array cannot be added or subtracted");
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)
                tkerr(current_token,"a structure cannot be added or subtracted");
            rv->type = get_arith_type(&rv->type, &rve.type);

            i2 = get_rval(&rve);
            t2 = rve.type;
            t = get_arith_type(&t1, &t2);
            add_cast_instr(i1, &t1, &t);
            add_cast_instr(i2, &t2, &t);
            if (tkop->code == ADD) {
                switch (t.type_base) {
                    case TB_INT:    add_instr(O_ADD_I); break;
                    case TB_DOUBLE: add_instr(O_ADD_D); break;
                    case TB_CHAR:   add_instr(O_ADD_C); break;
                }
            } else {
                switch (t.type_base) {
                    case TB_INT:    add_instr(O_SUB_I); break;
                    case TB_DOUBLE: add_instr(O_SUB_D); break;
                    case TB_CHAR:   add_instr(O_SUB_C); break;
                }
            }

            rv->is_ct_val = rv->is_lval = 0;

            if (exprAdd2(rv)) {
                return 1;
            }
        }
        else tkerr(current_token, "expected right operand");
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 1;
}

int exprAdd(ret_val_t *rv)
{
    token_t *start_token = current_token;
    instr_t *old_last_instr = last_instruction;

    if (exprMul(rv)) {
        if (exprAdd2(rv)) {
            return 1;
        }
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int exprMul2(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t rve;
    instr_t *i1, *i2;
    type_t t, t1, t2;
    instr_t *old_last_instr = last_instruction;

    if (consume(MUL) || consume(DIV)) {
        token_t *tkop = consumed_token;
        i1 = get_rval(rv);
        t1 = rv->type;

        if (exprCast(&rve)) {
            if(rv->type.num_elem > -1 || rve.type.num_elem> -1)
                    tkerr(current_token,"an array cannot be multiplied or divided");
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)
                    tkerr(current_token,"a structure cannot be multiplied or divided");
            rv->type = get_arith_type(&rv->type,&rve.type);

            i2 = get_rval(&rve);
            t2 = rve.type;
            t = get_arith_type(&t1, &t2);
            add_cast_instr(i1, &t1, &t);
            add_cast_instr(i2, &t2, &t);
            if (tkop->code == MUL) {
                switch (t.type_base) {
                    case TB_INT:    add_instr(O_MUL_I); break;
                    case TB_DOUBLE: add_instr(O_MUL_D); break;
                    case TB_CHAR:   add_instr(O_MUL_C); break;
                }
            } else {
                switch (t.type_base) {
                    case TB_INT:    add_instr(O_DIV_I); break;
                    case TB_DOUBLE: add_instr(O_DIV_D); break;
                    case TB_CHAR:   add_instr(O_DIV_C); break;
                }
            }

            rv->is_ct_val = rv->is_lval = 0;

            if (exprMul2(rv)) {
                return 1;
            }
        }
        else tkerr(current_token, "expected right operand");
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 1;
}

int exprMul(ret_val_t *rv)
{
    token_t *start_token = current_token;
    instr_t *old_last_instr = last_instruction;

    if (exprCast(rv)) {
        if (exprMul2(rv)) {
            return 1;
        }
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int exprCast(ret_val_t *rv)
{
    token_t *start_token = current_token;
    type_t t;
    ret_val_t rve;
    instr_t *old_last_instr = last_instruction;

    if (consume(LPAR)) {
        if (typeName(&t)) {
            if (consume(RPAR)) {
                if (exprCast(&rve)) {
                    cast(&t, &rve.type);

                    if (rv->type.num_elem < 0 && rv->type.type_base != TB_STRUCT) {
                        switch (rve.type.type_base) {
                            case TB_CHAR:
                                switch (t.type_base) {
                                    case TB_INT:    add_instr(O_CAST_C_I); break;
                                    case TB_DOUBLE: add_instr(O_CAST_C_D); break;
                                }
                                break;
                            case TB_DOUBLE:
                                switch (t.type_base) {
                                    case TB_CHAR: add_instr(O_CAST_D_C); break;
                                    case TB_INT:  add_instr(O_CAST_D_I); break;
                                }
                                break;
                            case TB_INT:
                                switch (t.type_base) {
                                    case TB_CHAR:   add_instr(O_CAST_I_C); break;
                                    case TB_DOUBLE: add_instr(O_CAST_I_D); break;
                                }
                                break;
                        }
                    }

                    rv->type = t;
                    rv->is_ct_val = rv->is_lval = 0;

                    return 1;
                }
                else tkerr(current_token, "missing expression to cast");
            }
            else tkerr(current_token, "missing )");
        }
        else tkerr(current_token, "expected type name after (");
    }

    delete_instructions_after(old_last_instr);
    if (exprUnary(rv)) {
        return 1;
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int exprUnary(ret_val_t *rv)
{
    token_t *start_token = current_token;
    instr_t *old_last_instr = last_instruction;

    if (consume(SUB) || consume(NOT)) {
        token_t *tkop = consumed_token;

        if (exprUnary(rv)) {
            if (tkop->code==SUB) {
                if (rv->type.num_elem >= 0)
                    tkerr(current_token, "unary '-' cannot be applied to an array");
                if (rv->type.type_base == TB_STRUCT)
                    tkerr(current_token, "unary '-' cannot be applied to a struct");

                get_rval(rv);
                switch (rv->type.type_base) {
                    case TB_CHAR:   add_instr(O_NEG_C); break;
                    case TB_INT:    add_instr(O_NEG_I); break;
                    case TB_DOUBLE: add_instr(O_NEG_D); break;
                }
            } else { // NOT
                if(rv->type.type_base == TB_STRUCT)
                    tkerr(current_token, "'!' cannot be applied to a struct");

                if (rv->type.num_elem < 0) {
                    get_rval(rv);
                    switch (rv->type.type_base) {
                        case TB_CHAR:   add_instr(O_NOT_C); break;
                        case TB_INT:    add_instr(O_NOT_I); break;
                        case TB_DOUBLE: add_instr(O_NOT_D); break;
                    }
                } else {
                    add_instr(O_NOT_A);
                }

                rv->type = create_type(TB_INT, -1);
            }
            rv->is_ct_val = rv->is_lval = 0;

            return 1;
        }
        else tkerr(current_token, "invalid expression");
    }
 
    if (exprPostfix(rv)) {
        return 1;
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int exprPostfix2(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t rve;
    instr_t *old_last_instr = last_instruction;

    if (consume(LBRACKET)) {
        if (expr(&rve)) {
            if(rv->type.num_elem < 0)
                tkerr(current_token, "only an array can be indexed");
            type_t type_int = create_type(TB_INT, -1);
            cast(&type_int, &rve.type);
            rv->type = rv->type;
            rv->type.num_elem = -1;
            rv->is_lval = 1;
            rv->is_ct_val = 0;

            if (consume(RBRACKET)) {
                add_cast_instr(last_instruction, &rve.type, &type_int);
                get_rval(&rve);
                if (type_base_size(&rv->type) != 1) {
                    add_instr_I(O_PUSHCT_I, type_base_size(&rv->type));
                    add_instr(O_MUL_I);
                }
                add_instr(O_OFFSET);

                if (exprPostfix2(rv)) {
                    return 1;
                }
            }
            else tkerr(current_token, "missing ]");
        }
        else tkerr(current_token, "missing expression after [");
    }
    if (consume(DOT)) {
        if (consume(ID)) {
            symbol_t *sStruct = rv->type.s;
            symbol_t *sMember = find_symbol(&sStruct->members, consumed_token->text);
            if(!sMember)
                tkerr(current_token, "struct %s does not have a member %s", sStruct->name, consumed_token->text);
            rv->type = sMember->type;
            rv->is_lval = 1;
            rv->is_ct_val = 0;

            if (sMember->offset) {
                add_instr_I(O_PUSHCT_I, sMember->offset);
                add_instr(O_OFFSET);
            }

            if (exprPostfix2(rv)) {
                return 1;
            }
        }
        else tkerr(current_token, "missing identifier after .");
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 1;
}

int exprPostfix(ret_val_t *rv)
{
    token_t *start_token = current_token;
    instr_t *old_last_instr = last_instruction;

    if (exprPrimary(rv)) {
        if (exprPostfix2(rv)) {
            return 1;
        }
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

int exprPrimary(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t arg;
    instr_t *i;
    instr_t *old_last_instr = last_instruction;

    if (consume(ID)) {
        token_t *tk_name = consumed_token;

        symbol_t *s = find_symbol(&symbols, tk_name->text);
        if(!s) tkerr(current_token, "undefined symbol %s", tk_name->text);
        rv->type = s->type;
        rv->is_ct_val = 0;
        rv->is_lval = 1;

        if (consume(LPAR)) {
            symbol_t **crt_def_arg = s->args.begin;
            if(s->cls != CLS_FUNC && s->cls != CLS_EXTFUNC)
                tkerr(current_token, "call of the non-function %s", tk_name->text);

            if (expr(&arg)) {
                if(crt_def_arg == s->args.end)
                    tkerr(current_token, "too many arguments in call");
                cast(&(*crt_def_arg)->type, &arg.type);

                if ((*crt_def_arg)->type.num_elem < 0) { // only arrays are passed by addr
                    i = get_rval(&arg);
                } else {
                    i = last_instruction;
                }
                add_cast_instr(i, &arg.type, &(*crt_def_arg)->type);

                crt_def_arg++;

                while (1) {
                    if (consume(COMMA)) {
                        if (expr(&arg)) {
                            if(crt_def_arg == s->args.end)
                                tkerr(current_token, "too many arguments in call");
                            cast(&(*crt_def_arg)->type, &arg.type);

                            if ((*crt_def_arg)->type.num_elem < 0) {
                                i = get_rval(&arg);
                            } else {
                                i = last_instruction;
                            }
                            add_cast_instr(i, &arg.type, &(*crt_def_arg)->type);

                            crt_def_arg++;
                        }
                        else tkerr(current_token, "missing expression after ,");
                    } else break;
                }
            }

            if (consume(RPAR)) {
                // Function call
                i = add_instr(s->cls == CLS_FUNC ? O_CALL : O_CALLEXT);
                i->args[0].addr = s->addr;

                if (crt_def_arg != s->args.end)
                    tkerr(current_token, "too few arguments in call");
                rv->type = s->type;
                rv->is_ct_val = rv->is_lval = 0;
            }
        }
        else {
            // Variable
            if (s->depth) {
                add_instr_I(O_PUSHFPADDR, s->offset);
            } else {
                add_instr_A(O_PUSHCT_A, s->addr);
            }

            if (s->cls == CLS_FUNC || s->cls == CLS_EXTFUNC)
                tkerr(current_token, "missing call for function %s", tk_name->text);
        }


        return 1;
    }
    else if (consume(CT_INT)) {
        token_t *tki = consumed_token;

        rv->type = create_type(TB_INT, -1);
        rv->ct_val.i = tki->i;
        rv->is_ct_val = 1;
        rv->is_lval = 0;

        add_instr_I(O_PUSHCT_I, tki->i);
        return 1;
    }
    else if (consume(CT_REAL)) {
        token_t *tkr = consumed_token;

        rv->type = create_type(TB_DOUBLE, -1);
        rv->ct_val.d = tkr->r;
        rv->is_ct_val = 1;
        rv->is_lval = 0;
 
        i = add_instr(O_PUSHCT_D);
        i->args[0].d = tkr->r;
        return 1;
    }
    else if (consume(CT_CHAR)) {
        token_t *tkc = consumed_token;

        rv->type = create_type(TB_CHAR, -1);
        rv->ct_val.i = tkc->i;
        rv->is_ct_val = 1;
        rv->is_lval = 0;
 
        add_instr_I(O_PUSHCT_C, tkc->i);
        return 1;
    }
    else if (consume(CT_STRING)) {
        token_t *tks = consumed_token;

        rv->type = create_type(TB_CHAR, 0);
        rv->ct_val.str = tks->text;
        rv->is_ct_val = 1;
        rv->is_lval = 0;
 
        add_instr_A(O_PUSHCT_A, tks->text);
        return 1;
    }
    else if (consume(LPAR)) {
        if (expr(rv)) {
            if (consume(RPAR)) {
                return 1;
            }
        }
    }

    delete_instructions_after(old_last_instr);
    current_token = start_token;
    return 0;
}

