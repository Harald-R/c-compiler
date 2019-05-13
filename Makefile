CC = gcc
CCOPT = -Wall -Wextra

APP ?= compiler
C_SOURCES = compiler.c common.c lexical_analyzer.c syntax_analyzer.c symbol_table.c type_analysis.c virtual_machine.c code_generation.c
C_HEADERS = common.h lexical_analyzer.h syntax_analyzer.h symbol_table.h type_analysis.h virtual_machine.h code_generation.h

.DEFAULT_GOAL: all

.PHONY: all
all: $(APP)

$(APP): $(C_SOURCES) $(C_HEADERS) Makefile
	$(CC) $(CCOPT) -o $@ $(C_SOURCES)

.PHONY: run
run: $(APP)
	./$(APP)

.PHONY: clean
clean:
	rm $(APP)
