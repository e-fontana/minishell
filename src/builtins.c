#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"
#include "output.h"

typedef struct {
    const char *name;
    void (*run)(char **argv, int *keep_running);
} Builtin;

static void builtin_exit(char **argv, int *keep_running)
{
    (void)argv;
    *keep_running = 0;
}

static void builtin_cd(char **argv, int *keep_running)
{
    (void)keep_running;
    if (argv[1] == NULL)
        write_str(STDERR_FILENO, "minishell: cd: informe o diretorio\n");
    else if (chdir(argv[1]) < 0)
        perror("minishell: cd");
}

static const Builtin builtins[] = {
    { "exit", builtin_exit },
    { "cd", builtin_cd },
};

int builtin_run(char **argv, int *keep_running)
{
    size_t count = sizeof(builtins) / sizeof(builtins[0]);

    for (size_t i = 0; i < count; i++) {
        if (strcmp(argv[0], builtins[i].name) == 0) {
            builtins[i].run(argv, keep_running);
            return 1;
        }
    }
    return 0;
}
