CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -Iinclude

SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)

minishell: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

build/%.o: src/%.c $(wildcard include/*.h) | build
	$(CC) $(CFLAGS) -c $< -o $@

build/test_parser: tests/test_parser.c src/parser.c include/parser.h | build
	$(CC) $(CFLAGS) -o $@ tests/test_parser.c src/parser.c

build:
	mkdir -p build

test: minishell build/test_parser
	./build/test_parser
	sh tests/scenarios.sh

clean:
	rm -rf build minishell

.PHONY: test clean
