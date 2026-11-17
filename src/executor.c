#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "executor.h"
#include "output.h"
#include "trace.h"

static void report_error(const char *message, const char *subject, int error_number)
{
    write_str(STDERR_FILENO, "minishell: ");
    write_str(STDERR_FILENO, message);
    if (subject != NULL)
        write_str(STDERR_FILENO, subject);
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
        report_error("nao foi possivel executar ", name, error);
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
    if (fd < 0 || fd == target)
        return;
    if (dup2(fd, target) < 0) {
        report_error("erro no dup2", NULL, errno);
        _exit(1);
    }
    trace("[trace] dup2(%d, %d)\n", fd, target);
    close(fd);
}

static void redirect_output(const char *path)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0) {
        report_error("nao foi possivel abrir ", path, errno);
        _exit(1);
    }
    move_fd(fd, STDOUT_FILENO);
}

static void run_child(const Command *cmd, const StageFds *fds)
{
    char *const *argv = cmd->argv;

    signal(SIGINT, SIG_DFL);
    move_fd(fds->input, STDIN_FILENO);
    move_fd(fds->output, STDOUT_FILENO);
    close_fd(fds->next_input);
    if (cmd->out_file != NULL)
        redirect_output(cmd->out_file);
    trace("[trace] execvp %s\n", argv[0]);
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
    trace("[trace] pipe -> %d %d\n", ends[0], ends[1]);
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
        trace("[trace] fork -> child %d\n", (int)pid);

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

    int decoded = WIFSIGNALED(status) ? 128 + WTERMSIG(status) : WEXITSTATUS(status);

    trace("[trace] wait %d -> status %d\n", (int)pid, decoded);
    if (WIFSIGNALED(status) && report_signal)
        write_str(STDERR_FILENO, "\nminishell: comando interrompido por sinal\n");
    return decoded;
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
