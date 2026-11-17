#include <errno.h>
#include <fcntl.h>
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

typedef struct {
    int input;
    int output;
    int next_input;
} StageFds;

static void close_fd(int fd)
{
    if (fd >= 0)
        close(fd);
}

static void close_stage_fds(const StageFds *fds)
{
    close_fd(fds->input);
    close_fd(fds->output);
    close_fd(fds->next_input);
}

static void move_fd(int fd, int target)
{
    if (fd < 0 || dup2(fd, target) >= 0)
        return;

    int error = errno;
    write_str(STDERR_FILENO, "minishell: erro no dup2: ");
    write_str(STDERR_FILENO, strerror(error));
    write_str(STDERR_FILENO, "\n");
    _exit(1);
}

static void redirect_output(const char *path)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0) {
        int error = errno;
        write_str(STDERR_FILENO, "minishell: nao foi possivel abrir ");
        write_str(STDERR_FILENO, path);
        write_str(STDERR_FILENO, ": ");
        write_str(STDERR_FILENO, strerror(error));
        write_str(STDERR_FILENO, "\n");
        _exit(1);
    }
    if (fd != STDOUT_FILENO) {
        move_fd(fd, STDOUT_FILENO);
        close(fd);
    }
}

static void run_child(const Command *cmd, const StageFds *fds)
{
    char *const *argv = cmd->argv;

    signal(SIGINT, SIG_DFL);
    move_fd(fds->input, STDIN_FILENO);
    move_fd(fds->output, STDOUT_FILENO);
    if (cmd->out_file != NULL)
        redirect_output(cmd->out_file);
    close_stage_fds(fds);
    execvp(argv[0], argv);

    int error = errno;
    report_exec_failure(argv[0], error);
    _exit(error == ENOENT ? 127 : 126);
}

static int open_stage_fds(StageFds *fds, int input, int is_last)
{
    int ends[2];

    fds->input = input;
    fds->output = -1;
    fds->next_input = -1;
    if (is_last)
        return 0;
    if (pipe(ends) < 0) {
        perror("minishell: erro no pipe");
        return -1;
    }
    fds->next_input = ends[0];
    fds->output = ends[1];
    return 0;
}

static int start_stages(const Pipeline *pipeline, pid_t *pids)
{
    int input = -1;
    int started = 0;

    for (int i = 0; i < pipeline->count; i++) {
        StageFds fds;

        if (open_stage_fds(&fds, input, i == pipeline->count - 1) < 0) {
            close_fd(input);
            break;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("minishell: erro no fork");
            close_stage_fds(&fds);
            break;
        }
        if (pid == 0)
            run_child(&pipeline->cmds[i], &fds);

        pids[started++] = pid;
        close_fd(fds.input);
        close_fd(fds.output);
        input = fds.next_input;
    }
    return started;
}

static int wait_child(pid_t pid, int report_signal)
{
    int status;

    if (waitpid(pid, &status, 0) < 0) {
        perror("minishell: erro no wait");
        return 1;
    }

    if (WIFSIGNALED(status)) {
        if (report_signal)
            write_str(STDERR_FILENO, "\nminishell: comando interrompido por sinal\n");
        return 128 + WTERMSIG(status);
    }
    return WEXITSTATUS(status);
}

int exec_pipeline(const Pipeline *pipeline)
{
    pid_t pids[MAX_CMDS];
    int started = start_stages(pipeline, pids);
    int last_status = 1;

    for (int i = 0; i < started; i++) {
        int is_last = i == pipeline->count - 1;
        int status = wait_child(pids[i], is_last);

        if (is_last)
            last_status = status;
    }
    return last_status;
}
