#include "lexical_analyzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    const char *file_path = "tests/8.c";
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

    p_crt_ch = inbuf;
    while(get_next_token() != END);

    print_atoms(tokens);

    free(inbuf);

    return 0;
}

