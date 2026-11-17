#ifndef BUILTINS_H
#define BUILTINS_H

#include "parser/parser.h"

int is_builtin(const char *name);
void report_builtin_misuse(void);
void builtin_run(const Command *cmd, int *keep_running);

#endif
