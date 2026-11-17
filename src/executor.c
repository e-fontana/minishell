#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "executor.h"
#include "output.h"

static void report_exec_failure(const char *name, int error)
{
    if (error == ENOENT) {
        write_str(STDERR_FILENO, "minishell: comando nao encontrado: ");
        write_str(STDERR_FILENO, name);
        write_str(STDERR_FILENO, "\n");
    } else {
        write_str(STDERR_FILENO, "minishell: nao foi possivel executar ");
        write_str(STDERR_FILENO, name);
        write_str(STDERR_FILENO, ": ");
        write_str(STDERR_FILENO, strerror(error));
        write_str(STDERR_FILENO, "\n");
    }
}

static void run_child(char *const *argv)
{
    signal(SIGINT, SIG_DFL);
    execvp(argv[0], argv);

    int error = errno;
    report_exec_failure(argv[0], error);
    _exit(error == ENOENT ? 127 : 126);
}

static int run_command(const Command *cmd)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("minishell: erro no fork");
        return 1;
    }

    if (pid == 0)
        run_child(cmd->argv);

    int status;

    if (waitpid(pid, &status, 0) < 0) {
        perror("minishell: erro no wait");
        return 1;
    }

    if (WIFSIGNALED(status)) {
        write_str(STDERR_FILENO, "\nminishell: comando interrompido por sinal\n");
        return 128 + WTERMSIG(status);
    }
    return WEXITSTATUS(status);
}

int exec_pipeline(const Pipeline *pipeline)
{
    return run_command(&pipeline->cmds[0]);
}
