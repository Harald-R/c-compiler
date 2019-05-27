#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "lexical_analyzer.h"
#include "syntax_analyzer.h"
#include "symbol_table.h"
#include "virtual_machine.h"
#include "optimization.h"

double timer()
{
    return (double)clock() / (double)CLOCKS_PER_SEC;
}

void mvTest()
{
    instr_t *L1;
    int *v = alloc_global(sizeof(long int));
    add_instr_A(O_PUSHCT_A,v);
    add_instr_I(O_PUSHCT_I,3);
    add_instr_I(O_STORE,sizeof(long int));
    L1=add_instr_A(O_PUSHCT_A,v);
    add_instr_I(O_LOAD,sizeof(long int));
    add_instr_A(O_CALLEXT,require_symbol(&symbols,"put_i")->addr);
    add_instr_A(O_PUSHCT_A,v);
    add_instr_A(O_PUSHCT_A,v);
    add_instr_I(O_LOAD,sizeof(long int));
    add_instr_I(O_PUSHCT_I,1);
    add_instr(O_SUB_I);
    add_instr_I(O_STORE,sizeof(long int));
    add_instr_A(O_PUSHCT_A,v);
    add_instr_I(O_LOAD,sizeof(long int));
    add_instr_A(O_JT_I,L1);
    add_instr(O_HALT);
}

int main(int argc, char *argv[])
{
    const char *file_path = "tests/10.c";
    if (argc > 1) {
        file_path = argv[1];
    }

    unsigned int optimize_code = 0;
    if (argc > 2) {
        if (strcmp(argv[2], "-O") == 0)
            optimize_code = 1;
    }

    struct stat stat_buf;
    if (stat(file_path, &stat_buf) < 0) {
        perror("Error opening file");
        exit(1);
    }

    unsigned int file_size = stat_buf.st_size;
    char *inbuf = (char *) malloc((file_size + 1) * sizeof(char));
    if (!inbuf) {
        perror("Error allocating memory");
        exit(1);
    }

    FILE *f;
    if ((f = fopen(file_path, "r")) == NULL) {
        perror("Error opening file");
        exit(1);
    }

    int n = fread(inbuf, sizeof(char), file_size, f);
    if (n == 0) {
        perror("Error reading from file");
        exit(1);
    }
    inbuf[n] = '\0';
    fclose(f);

    // Lexical analysis
    get_atoms(inbuf);
    // print_atoms(tokens);

    // Free input buffer
    free(inbuf);

    // Analyze syntax and generate symbol table
    current_token = tokens;

    init_symbol_table(&symbols);
    if (analyze_syntax()) {
        printf("Syntax OK\n");
    } else {
        printf("Syntax ERROR\n");
        return 1;
    }

    // Print symbol table
    printf("\nSymbol table:\n");
    print_symbol_table(&symbols);

    // Optimize code if option enabled
    if (optimize_code)
        optimize();

    // Run code generator
    double t1, t2;
    unsigned int executed_instr = 0;

    t1 = timer();
    executed_instr = run(instructions);
    t2 = timer();

    printf("\nTime unopt: %g sec (%d executed instruction)\n", t2-t1, executed_instr);

    return 0;
}

