CC = gcc
CCOPT = -Wall -Wextra

APP ?= compiler
C_SOURCES = compiler.c lexical_analyzer.c syntax_analyzer.c

.DEFAULT_GOAL: all

.PHONY: all
all: $(APP)

$(APP): $(C_SOURCES) Makefile
	$(CC) $(CCOPT) -o $@ $(C_SOURCES)

.PHONY: run
run: $(APP)
	./$(APP)

.PHONY: clean
clean:
	rm $(APP)
