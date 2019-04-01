#include "syntax_analyzer.h"

int analyze_syntax()
{
    return unit();
}

int consume(int code)
{
    if (current_token->code == code) {
        current_token = current_token->next;
        return 1;
    }
    return 0;
}

int unit()
{
    return 0;
}

int expr()
{
    return 0;
}

int stmt()
{
    return 0;
}

int rule_while()
{
    token *start_token = current_token;
    if (consume(WHILE)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (stmt()) {
                        return 1;
                    }
                    else tkerr(current_token, ERR_MISSING_STMT("while"));
                }
                else tkerr(current_token, ERR_MISSING_RPAR);
            }
            else tkerr(current_token, ERR_INVALID_EXPR);
        }
        else tkerr(current_token, ERR_MISSING_LPAR("while"));
    }

    current_token = start_token;
    return 0;
}

