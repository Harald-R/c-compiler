#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "lexical_analyzer.h"
#include "syntax_analyzer.h"
#include "symbol_table.h"

int main(int argc, char *argv[])
{
    const char *file_path = "tests/9.c";
    if (argc > 1) {
        file_path = argv[1];
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

    return 0;
}

