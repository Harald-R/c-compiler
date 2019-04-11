#include <stdio.h>
#include "syntax_analyzer.h"
#include "domain_analyzer.h"

token *consumed_token;

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
    token *start_token = current_token;
    token *symbol_token;

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
    token *start_token = current_token;
    token *symbol_token;
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
    token *start_token = current_token;
    token *symbol_token;
    type_t t;
    int is_decl_var = 0;

    if (typeBase(&t)) {
        if (consume(ID)) {
            symbol_token = consumed_token;

            if (arrayDecl(&t)) {
                is_decl_var = 1;
                t.num_elem = -1;
            }

            add_var(symbol_token, &t);

            while (1) {
                if (consume(COMMA)) {
                    is_decl_var = 1;
                    if (consume(ID)) {
                        symbol_token = consumed_token;

                        if (arrayDecl(&t)) {
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
    token *start_token = current_token;

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
            token *symbol_token = consumed_token;
            
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
    token *start_token = current_token;

    if (typeBase(ret)) {
        if (arrayDecl(ret)) {
            ret->num_elem = -1;
        }

        return 1;
    }

    current_token = start_token;
    return 0;

}

int arrayDecl(type_t *ret)
{
    token *start_token = current_token;

    if (consume(LBRACKET)) {
        ret->num_elem = 0;
        if (expr()) { }

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
    token *start_token = current_token;
    token *symbol_token;
    type_t t;

    if (typeBase(&t)) {
        if (consume(ID)) {
            symbol_token = consumed_token;

            if (arrayDecl(&t)) {
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
    token *start_token = current_token;

    if (stmCompound()) return 1;
    else if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr()) {
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
            if (expr()) {
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
        if (consume(LPAR)) {
            if (expr()) { }
            if (consume(SEMICOLON)) {
                if (expr()) { }
                if (consume(SEMICOLON)) {
                    if (expr()) { }
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
        if (expr()) { }
        if (consume(SEMICOLON)) {
            return 1;
        }
        else tkerr(current_token, "missing semicolon");
    }
    else if (expr()) {
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
    token *start_token = current_token;
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

int expr()
{
    token *start_token = current_token;

    if (exprAssign()) {
        return 1;
    }

    current_token = start_token;
    return 0;
}

int exprAssign()
{
    token *start_token = current_token;

    if (exprUnary()) {
        if (consume(ASSIGN)) {
            if (exprAssign()) {
                return 1;
            }
            else tkerr(current_token, "missing value after = operator");
        }
    }

    current_token = start_token;
    if (exprOr()) {
        return 1;
    }

    current_token = start_token;
    return 0;
}

int exprOr2()
{
    token *start_token = current_token;

    if (consume(OR)) {
        if (exprAnd()) {
            if (exprOr2()) {
                return 1;
            }
        }
        else tkerr(current_token, "missing expression after ||");
    }

    current_token = start_token;
    return 1;
}

int exprOr()
{
    token *start_token = current_token;

    if (exprAnd()) {
        if (exprOr2()) {
            return 1;
        }
    }

    current_token = start_token;
    return 0;
}

int exprAnd2()
{
    token *start_token = current_token;

    if (consume(AND)) {
        if (exprEq()) {
            if (exprAnd2()) {
                return 1;
            }
        }
        else tkerr(current_token, "missing expression after &&");
    }

    current_token = start_token;
    return 1;
}

int exprAnd()
{
    token *start_token = current_token;

    if (exprEq()) {
        if (exprAnd2()) {
            return 1;
        }
    }

    current_token = start_token;
    return 0;
}

int exprEq2()
{
    token *start_token = current_token;

    if (consume(EQUAL) || consume(NOTEQ)) {
        if (exprRel()) {
            if (exprEq2()) {
                return 1;
            }
        }
        else tkerr(current_token, "missing expression after equality comparison");
    }

    current_token = start_token;
    return 1;
}

int exprEq()
{
    token *start_token = current_token;

    if (exprRel()) {
        if (exprEq2()) {
            return 1;
        }
    }

    current_token = start_token;
    return 0;
}

int exprRel2()
{
    token *start_token = current_token;

    if (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)) {
        if (exprAdd()) {
            if (exprRel2()) {
                return 1;
            }
        }
        else tkerr(current_token, "missing expression after comparison");
    }

    current_token = start_token;
    return 1;
}

int exprRel()
{
    token *start_token = current_token;

    if (exprAdd()) {
        if (exprRel2()) {
            return 1;
        }
    }

    current_token = start_token;
    return 0;
}

int exprAdd2()
{
    token *start_token = current_token;

    if (consume(ADD) || consume(SUB)) {
        if (exprMul()) {
            if (exprAdd2()) {
                return 1;
            }
        }
        else tkerr(current_token, "expected right operand");
    }

    current_token = start_token;
    return 1;
}

int exprAdd()
{
    token *start_token = current_token;

    if (exprMul()) {
        if (exprAdd2()) {
            return 1;
        }
    }

    current_token = start_token;
    return 0;
}

int exprMul2()
{
    token *start_token = current_token;

    if (consume(MUL) || consume(DIV)) {
        if (exprCast()) {
            if (exprMul2()) {
                return 1;
            }
        }
        else tkerr(current_token, "expected right operand");
    }

    current_token = start_token;
    return 1;
}

int exprMul()
{
    token *start_token = current_token;

    if (exprCast()) {
        if (exprMul2()) {
            return 1;
        }
    }

    current_token = start_token;
    return 0;
}

int exprCast()
{
    token *start_token = current_token;
    type_t t;

    if (consume(LPAR)) {
        if (typeName(&t)) {
            if (consume(RPAR)) {
                if (exprCast()) {
                    return 1;
                }
                else tkerr(current_token, "missing expression to cast");
            }
            else tkerr(current_token, "missing )");
        }
        else tkerr(current_token, "expected type name after (");
    }
    
    if (exprUnary()) {
        return 1;
    }

    current_token = start_token;
    return 0;
}

int exprUnary()
{
    token *start_token = current_token;

    if (consume(SUB) || consume(NOT)) {
        if (exprUnary()) {
            return 1;
        }
        else tkerr(current_token, "invalid expression");
    }
 
    if (exprPostfix()) {
        return 1;
    }

    current_token = start_token;
    return 0;
}

int exprPostfix2()
{
    token *start_token = current_token;

    if (consume(LBRACKET)) {
        if (expr()) {
            if (consume(RBRACKET)) {
                if (exprPostfix2()) {
                    return 1;
                }
            }
            else tkerr(current_token, "missing ]");
        }
        else tkerr(current_token, "missing expression after [");
    }
    if (consume(DOT)) {
        if (consume(ID)) {
            if (exprPostfix2()) {
                return 1;
            }
        }
        else tkerr(current_token, "missing identifier after .");
    }

    current_token = start_token;
    return 1;
}

int exprPostfix()
{
    token *start_token = current_token;

    if (exprPrimary()) {
        if (exprPostfix2()) {
            return 1;
        }
    }

    current_token = start_token;
    return 0;
}

int exprPrimary()
{
    token *start_token = current_token;

    if (consume(ID)) {
        if (consume(LPAR)) {
            if (expr()) {
                while (1) {
                    if (consume(COMMA)) {
                        if (expr()) { }
                        else tkerr(current_token, "missing expression after ,");
                    } else break;
                }
            }

            if (consume(RPAR)) { }
        }

        return 1;
    }
    else if (consume(CT_INT) || consume(CT_REAL) || consume(CT_CHAR) || consume(CT_STRING)) {
        return 1;
    }
    else if (consume(LPAR)) {
        if (expr()) {
            if (consume(RPAR)) {
                return 1;
            }
        }
    }

    current_token = start_token;
    return 0;
}


