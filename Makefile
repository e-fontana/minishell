CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -Iinclude

SRC = $(wildcard src/*.c src/*/*.c)
OBJ = $(SRC:src/%.c=build/%.o)
HDR = $(wildcard include/*/*.h)

all: minishell

minishell: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

build/%.o: src/%.c $(HDR)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

build/test_parser: tests/test_parser.c src/parser/parser.c include/parser/parser.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ tests/test_parser.c src/parser/parser.c

test: minishell build/test_parser
	./build/test_parser
	sh tests/scenarios.sh

clean:
	rm -rf build minishell

.PHONY: all test clean
