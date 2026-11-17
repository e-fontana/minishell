#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "exec/builtins.h"
#include "io/output.h"

typedef struct {
    const char *name;
    void (*run)(char *const *argv, int *keep_running);
} Builtin;

static void builtin_exit(char *const *argv, int *keep_running)
{
    (void)argv;
    *keep_running = 0;
}

static void builtin_cd(char *const *argv, int *keep_running)
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

static const Builtin *find_builtin(const char *name)
{
    size_t count = sizeof(builtins) / sizeof(builtins[0]);

    for (size_t i = 0; i < count; i++)
        if (strcmp(name, builtins[i].name) == 0)
            return &builtins[i];
    return NULL;
}

int is_builtin(const char *name)
{
    return find_builtin(name) != NULL;
}

void report_builtin_misuse(void)
{
    write_str(STDERR_FILENO, "minishell: comando interno nao pode ser usado em pipeline ou com redirecionamento\n");
}

void builtin_run(const Command *cmd, int *keep_running)
{
    const Builtin *builtin = find_builtin(cmd->argv[0]);

    if (builtin != NULL)
        builtin->run(cmd->argv, keep_running);
}
