CC = gcc
CCOPT = -Wall -Wextra

APP ?= compiler

INC_DIR = include
SRC_DIR = src

C_HEADERS := $(shell find $(INC_DIR) -name '*.h')
C_SOURCES := $(shell find $(SRC_DIR) -name '*.c')

.DEFAULT_GOAL: all

.PHONY: all
all: $(APP)

$(APP): $(C_SOURCES) $(C_HEADERS) Makefile
	$(CC) $(CCOPT) -I $(INC_DIR) -o $@ $(C_SOURCES)

.PHONY: run
run: $(APP)
	./$(APP)

.PHONY: clean
clean:
	rm $(APP)
