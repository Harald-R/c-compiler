#include <stdio.h>
#include "syntax_analyzer.h"
#include "symbol_table.h"
#include "type_analysis.h"

token_t *consumed_token;

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
    while (declStruct() || declFunc() || declVar());
    if (consume(END)) {
        return 1;
    }
    else tkerr(current_token, "invalid expression");

    return 0;
}

int declStruct()
{
    token_t *start_token = current_token;
    token_t *symbol_token;

    if (consume(STRUCT)) {
        if (consume(ID)) {
            symbol_token = consumed_token;

            if (consume(LACC)) {
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

    current_token = start_token;
    return 0;
}

int declFunc()
{
    token_t *start_token = current_token;
    token_t *symbol_token;
    type_t t;
    int is_decl_func = 0;
    int has_void_return_type = 0;

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

                    if (stmCompound()) {
                        delete_symbols_after(&symbols, current_func);
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

    current_token = start_token;
    return 0;
}

int declVar()
{
    token_t *start_token = current_token;
    token_t *symbol_token;
    type_t t;
    int is_decl_var = 0;

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

    current_token = start_token;
    return 0;
}

int typeBase(type_t *ret)
{
    token_t *start_token = current_token;

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

    current_token = start_token;
    return 0;
}

int typeName(type_t *ret)
{
    token_t *start_token = current_token;

    if (typeBase(ret)) {
        if (arrayDecl(ret)) { }
        else {
            ret->num_elem = -1;
        }

        return 1;
    }

    current_token = start_token;
    return 0;

}

int arrayDecl(type_t *ret)
{
    token_t *start_token = current_token;
    ret_val_t rv;

    if (consume(LBRACKET)) {
        ret->num_elem = 0;

        if (expr(&rv)) {
            if (!rv.is_ct_val)
                tkerr(current_token, "the array size is not a constant");
            if (rv.type.type_base != TB_INT)
                tkerr(current_token, "the array size is not an integer");

            ret->num_elem = rv.ct_val.i;
        }

        if (consume(RBRACKET)) {
            return 1;
        }
        else tkerr(current_token, "missing ]");
    }

    current_token = start_token;
    return 0;
}

int funcArg()
{
    token_t *start_token = current_token;
    token_t *symbol_token;
    type_t t;

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
            
            s = add_symbol(&current_func->args, symbol_token->text, CLS_VAR);
            s->mem = MEM_ARG;
            s->type = t;

            return 1;
        }
        else tkerr(current_token, "missing argument identifier");
    }

    current_token = start_token;
    return 0;
}

int stm()
{
    token_t *start_token = current_token;
    ret_val_t rv;

    if (stmCompound()) return 1;
    else if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr(&rv)) {
                if (rv.type.type_base == TB_STRUCT)
                    tkerr(current_token, "a structure cannot be logically tested");

                if (consume(RPAR)) {
                    if (stm()) {
                        if (consume(ELSE)) {
                            if (stm()) { }
                        }

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
        if (consume(LPAR)) {
            if (expr(&rv)) {
                if (rv.type.type_base == TB_STRUCT)
                    tkerr(current_token, "a structure cannot be logically tested");

                if (consume(RPAR)) {
                    if (stm()) {
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

        if (consume(LPAR)) {
            if (expr(&rv1)) { }
            if (consume(SEMICOLON)) {
                if (expr(&rv2)) {
                    if(rv2.type.type_base == TB_STRUCT)
                        tkerr(current_token, "a structure cannot be logically tested");
                }

                if (consume(SEMICOLON)) {
                    if (expr(&rv3)) { }
                    if (consume(RPAR)) {
                        if (stm()) {
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
            return 1;
        }
        else tkerr(current_token, "missing semicolon");
    }
    else if (consume(RETURN)) {
        if (expr(&rv)) {
            if(current_func->type.type_base == TB_VOID)
                tkerr(current_token, "a void function cannot return a value");
            cast(&current_func->type, &rv.type);
        }

        if (consume(SEMICOLON)) {
            return 1;
        }
        else tkerr(current_token, "missing semicolon");
    }
    else if (expr(&rv)) {
        if (consume(SEMICOLON)) {
            return 1;
        }
        else tkerr(current_token, "expected semicolon");
    }
    else if (consume(SEMICOLON)) {
        return 1;
    }

    current_token = start_token;
    return 0;
}

int stmCompound()
{
    token_t *start_token = current_token;
    symbol_t *start = symbols.end[-1];

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

    current_token = start_token;
    return 0;
}

int expr(ret_val_t *rv)
{
    token_t *start_token = current_token;

    if (exprAssign(rv)) {
        return 1;
    }

    current_token = start_token;
    return 0;
}

int exprAssign(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t rve;

    if (exprUnary(rv)) {
        if (consume(ASSIGN)) {
            if (exprAssign(&rve)) {
                if (!rv->is_lval)
                    tkerr(current_token, "cannot assign to a non-lval");
                if (rv->type.num_elem > -1 || rve.type.num_elem > -1)
                    tkerr(current_token, "the arrays cannot be assigned");

                cast(&rv->type, &rve.type);
                rv->is_ct_val = rv->is_lval = 0;

                return 1;
            }
            else tkerr(current_token, "missing value after = operator");
        }
    }

    current_token = start_token;
    if (exprOr(rv)) {
        return 1;
    }

    current_token = start_token;
    return 0;
}

int exprOr2(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t rve;

    if (consume(OR)) {
        if (exprAnd(&rve)) {
            if (rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)
                    tkerr(current_token, "a structure cannot be logically tested");
            rv->type = create_type(TB_INT, -1);
            rv->is_ct_val = rv->is_lval = 0;

            if (exprOr2(rv)) {
                return 1;
            }
        }
        else tkerr(current_token, "missing expression after ||");
    }

    current_token = start_token;
    return 1;
}

int exprOr(ret_val_t *rv)
{
    token_t *start_token = current_token;

    if (exprAnd(rv)) {
        if (exprOr2(rv)) {
            return 1;
        }
    }

    current_token = start_token;
    return 0;
}

int exprAnd2(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t rve;

    if (consume(AND)) {
        if (exprEq(&rve)) {
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)
                tkerr(current_token, "a structure cannot be logically tested");
            rv->type = create_type(TB_INT, -1);
            rv->is_ct_val = rv->is_lval = 0;

            if (exprAnd2(rv)) {
                return 1;
            }
        }
        else tkerr(current_token, "missing expression after &&");
    }

    current_token = start_token;
    return 1;
}

int exprAnd(ret_val_t *rv)
{
    token_t *start_token = current_token;

    if (exprEq(rv)) {
        if (exprAnd2(rv)) {
            return 1;
        }
    }

    current_token = start_token;
    return 0;
}

int exprEq2(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t rve;

    if (consume(EQUAL) || consume(NOTEQ)) {
        if (exprRel(&rve)) {
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)
                tkerr(current_token,"a structure cannot be compared");
            rv->type = create_type(TB_INT, -1);
            rv->is_ct_val = rv->is_lval = 0;

            if (exprEq2(rv)) {
                return 1;
            }
        }
        else tkerr(current_token, "missing expression after equality comparison");
    }

    current_token = start_token;
    return 1;
}

int exprEq(ret_val_t *rv)
{
    token_t *start_token = current_token;

    if (exprRel(rv)) {
        if (exprEq2(rv)) {
            return 1;
        }
    }

    current_token = start_token;
    return 0;
}

int exprRel2(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t rve;

    if (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)) {
        if (exprAdd(&rve)) {
            if(rv->type.num_elem > -1 || rve.type.num_elem > -1)
                tkerr(current_token,"an array cannot be compared");
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)
                tkerr(current_token,"a structure cannot be compared");
            rv->type = create_type(TB_INT, -1);
            rv->is_ct_val = rv->is_lval = 0;

            if (exprRel2(rv)) {
                return 1;
            }
        }
        else tkerr(current_token, "missing expression after comparison");
    }

    current_token = start_token;
    return 1;
}

int exprRel(ret_val_t *rv)
{
    token_t *start_token = current_token;

    if (exprAdd(rv)) {
        if (exprRel2(rv)) {
            return 1;
        }
    }

    current_token = start_token;
    return 0;
}

int exprAdd2(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t rve;

    if (consume(ADD) || consume(SUB)) {
        if (exprMul(&rve)) {
            if(rv->type.num_elem > -1 || rve.type.num_elem > -1)
                tkerr(current_token,"an array cannot be added or subtracted");
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)
                tkerr(current_token,"a structure cannot be added or subtracted");
            rv->type = get_arith_type(&rv->type, &rve.type);
            rv->is_ct_val = rv->is_lval = 0;

            if (exprAdd2(rv)) {
                return 1;
            }
        }
        else tkerr(current_token, "expected right operand");
    }

    current_token = start_token;
    return 1;
}

int exprAdd(ret_val_t *rv)
{
    token_t *start_token = current_token;

    if (exprMul(rv)) {
        if (exprAdd2(rv)) {
            return 1;
        }
    }

    current_token = start_token;
    return 0;
}

int exprMul2(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t rve;

    if (consume(MUL) || consume(DIV)) {
        if (exprCast(&rve)) {
            if(rv->type.num_elem > -1 || rve.type.num_elem> -1)
                    tkerr(current_token,"an array cannot be multiplied or divided");
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)
                    tkerr(current_token,"a structure cannot be multiplied or divided");
            rv->type = get_arith_type(&rv->type,&rve.type);
            rv->is_ct_val = rv->is_lval = 0;

            if (exprMul2(rv)) {
                return 1;
            }
        }
        else tkerr(current_token, "expected right operand");
    }

    current_token = start_token;
    return 1;
}

int exprMul(ret_val_t *rv)
{
    token_t *start_token = current_token;

    if (exprCast(rv)) {
        if (exprMul2(rv)) {
            return 1;
        }
    }

    current_token = start_token;
    return 0;
}

int exprCast(ret_val_t *rv)
{
    token_t *start_token = current_token;
    type_t t;
    ret_val_t rve;

    if (consume(LPAR)) {
        if (typeName(&t)) {
            if (consume(RPAR)) {
                if (exprCast(&rve)) {
                    cast(&t, &rve.type);
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
    
    if (exprUnary(rv)) {
        return 1;
    }

    current_token = start_token;
    return 0;
}

int exprUnary(ret_val_t *rv)
{
    token_t *start_token = current_token;

    if (consume(SUB) || consume(NOT)) {
        token_t *tkop = consumed_token;

        if (exprUnary(rv)) {
            if (tkop->code==SUB) {
                if (rv->type.num_elem >= 0)
                    tkerr(current_token, "unary '-' cannot be applied to an array");
                if (rv->type.type_base == TB_STRUCT)
                    tkerr(current_token, "unary '-' cannot be applied to a struct");
            } else { // NOT
                if(rv->type.type_base == TB_STRUCT)
                    tkerr(current_token, "'!' cannot be applied to a struct");
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

    current_token = start_token;
    return 0;
}

int exprPostfix2(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t rve;

    if (consume(LBRACKET)) {
        if (expr(&rve)) {
            if(rv->type.num_elem < 0)
                tkerr(current_token, "only an array can be indexed");
            type_t typeInt = create_type(TB_INT, -1);
            cast(&typeInt, &rve.type);
            rv->type = rv->type;
            rv->type.num_elem = -1;
            rv->is_lval = 1;
            rv->is_ct_val = 0;

            if (consume(RBRACKET)) {
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

            if (exprPostfix2(rv)) {
                return 1;
            }
        }
        else tkerr(current_token, "missing identifier after .");
    }

    current_token = start_token;
    return 1;
}

int exprPostfix(ret_val_t *rv)
{
    token_t *start_token = current_token;

    if (exprPrimary(rv)) {
        if (exprPostfix2(rv)) {
            return 1;
        }
    }

    current_token = start_token;
    return 0;
}

int exprPrimary(ret_val_t *rv)
{
    token_t *start_token = current_token;
    ret_val_t arg;

    if (consume(ID)) {
        token_t *tk_name = consumed_token;

        symbol_t *s = find_symbol(&symbols, tk_name->text);
        if(!s) tkerr(current_token, "undefined symbol %s", tk_name->text);
        rv->type = s->type;
        rv->is_ct_val = 0;
        rv->is_lval = 1;

        if (consume(LPAR)) {
            symbol_t **crtDefArg = s->args.begin;
            if(s->cls != CLS_FUNC && s->cls != CLS_EXTFUNC)
                tkerr(current_token, "call of the non-function %s", tk_name->text);

            if (expr(&arg)) {
                if(crtDefArg == s->args.end)
                    tkerr(current_token, "too many arguments in call");
                cast(&(*crtDefArg)->type, &arg.type);
                crtDefArg++;

                while (1) {
                    if (consume(COMMA)) {
                        if (expr(&arg)) {
                            if(crtDefArg == s->args.end)
                                tkerr(current_token, "too many arguments in call");
                            cast(&(*crtDefArg)->type, &arg.type);
                            crtDefArg++;
                        }
                        else tkerr(current_token, "missing expression after ,");
                    } else break;
                }
            }

            if (consume(RPAR)) {
                if(crtDefArg != s->args.end)
                    tkerr(current_token, "too few arguments in call");
                rv->type = s->type;
                rv->is_ct_val = rv->is_lval = 0;
            }
            else {
                if(s->cls == CLS_FUNC || s->cls == CLS_EXTFUNC)
                    tkerr(current_token, "missing call for function %s", tk_name->text);
            }
        }

        return 1;
    }
    else if (consume(CT_INT)) {
        token_t *tki = consumed_token;

        rv->type = create_type(TB_INT, -1);
        rv->ct_val.i = tki->i;
        rv->is_ct_val = 1;
        rv->is_lval = 0;
        return 1;
    }
    else if (consume(CT_REAL)) {
        token_t *tkr = consumed_token;

        rv->type = create_type(TB_DOUBLE, -1);
        rv->ct_val.d = tkr->r;
        rv->is_ct_val = 1;
        rv->is_lval = 0;
        return 1;
    }
    else if (consume(CT_CHAR)) {
        token_t *tkc = consumed_token;

        rv->type = create_type(TB_CHAR, -1);
        rv->ct_val.i = tkc->i;
        rv->is_ct_val = 1;
        rv->is_lval = 0;
        return 1;
    }
    else if (consume(CT_STRING)) {
        token_t *tks = consumed_token;

        rv->type = create_type(TB_CHAR, 0);
        rv->ct_val.str = tks->text;
        rv->is_ct_val = 1;
        rv->is_lval = 0;
        return 1;
    }
    else if (consume(LPAR)) {
        if (expr(rv)) {
            if (consume(RPAR)) {
                return 1;
            }
        }
    }

    current_token = start_token;
    return 0;
}

