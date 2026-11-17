#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "executor.h"
#include "output.h"

static void report_error(const char *action, const char *subject, int error_number)
{
    write_str(STDERR_FILENO, "minishell: ");
    write_str(STDERR_FILENO, action);
    if (subject != NULL) {
        write_str(STDERR_FILENO, " ");
        write_str(STDERR_FILENO, subject);
    }
    write_str(STDERR_FILENO, ": ");
    write_str(STDERR_FILENO, strerror(error_number));
    write_str(STDERR_FILENO, "\n");
}

static void report_exec_failure(const char *name, int error)
{
    if (error == ENOENT) {
        write_str(STDERR_FILENO, "minishell: comando nao encontrado: ");
        write_str(STDERR_FILENO, name);
        write_str(STDERR_FILENO, "\n");
    } else {
        report_error("nao foi possivel executar", name, error);
    }
}

static void redirect_output(const char *path)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0) {
        report_error("nao foi possivel abrir", path, errno);
        _exit(1);
    }
    if (fd == STDOUT_FILENO)
        return;
    if (dup2(fd, STDOUT_FILENO) < 0) {
        report_error("erro no dup2", NULL, errno);
        _exit(1);
    }
    close(fd);
}

static void run_child(const Command *cmd)
{
    signal(SIGINT, SIG_DFL);
    if (cmd->out_file != NULL)
        redirect_output(cmd->out_file);
    execvp(cmd->argv[0], cmd->argv);

    int error = errno;
    report_exec_failure(cmd->argv[0], error);
    _exit(error == ENOENT ? 127 : 126);
}

int exec_command(const Command *cmd)
{
    int status;
    pid_t pid = fork();

    if (pid < 0) {
        report_error("erro no fork", NULL, errno);
        return 1;
    }
    if (pid == 0)
        run_child(cmd);

    if (wait(&status) < 0) {
        report_error("erro no wait", NULL, errno);
        return 1;
    }
    if (WIFSIGNALED(status)) {
        write_str(STDERR_FILENO, "\nminishell: comando interrompido por sinal\n");
        return 128 + WTERMSIG(status);
    }
    return WEXITSTATUS(status);
}
