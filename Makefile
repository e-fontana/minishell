CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -Iinclude

SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)

minishell: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

build/%.o: src/%.c include/minishell.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

test: minishell
	sh tests/scenarios.sh

clean:
	rm -rf build minishell

.PHONY: test clean
