#include <signal.h>
#include <unistd.h>

#include "builtins.h"
#include "executor.h"
#include "input.h"
#include "output.h"
#include "parser.h"

int main(void)
{
    char line[LINE_SIZE];
    char *argv[MAX_ARGS];
    int keep_running = 1;

    signal(SIGINT, SIG_IGN);

    while (keep_running) {
        write_str(STDOUT_FILENO, "> ");

        ReadStatus read_status = read_line(line, LINE_SIZE);
        if (read_status == READ_EOF) {
            write_str(STDOUT_FILENO, "\n");
            break;
        }
        if (read_status == READ_TOO_LONG) {
            write_str(STDERR_FILENO, "minishell: linha muito longa\n");
            continue;
        }

        int word_count = split_words(line, argv, MAX_ARGS);
        if (word_count < 0) {
            write_str(STDERR_FILENO, "minishell: argumentos demais\n");
            continue;
        }
        if (word_count == 0)
            continue;

        if (builtin_run(argv, &keep_running))
            continue;

        exec_command(argv);
    }

    write_str(STDOUT_FILENO, "Encerrando o minishell.\n");
    return 0;
}
