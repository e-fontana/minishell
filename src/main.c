#include <signal.h>
#include <unistd.h>

#include "exec/builtins.h"
#include "exec/executor.h"
#include "io/input.h"
#include "io/output.h"
#include "parser/parser.h"

static void report_parse_error(ParseStatus status)
{
    if (status == PARSE_TOO_MANY_ARGS)
        write_str(STDERR_FILENO, "minishell: argumentos demais\n");
    else if (status == PARSE_SYNTAX_ERROR)
        write_str(STDERR_FILENO, "minishell: erro de sintaxe\n");
}

int main(void)
{
    char line[LINE_SIZE];
    Command cmd;
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

        ParseStatus parse_status = parse_line(line, &cmd);
        if (parse_status != PARSE_OK) {
            report_parse_error(parse_status);
            continue;
        }

        if (is_builtin(cmd.argv[0])) {
            if (cmd.out_file == NULL)
                builtin_run(&cmd, &keep_running);
            else
                report_builtin_misuse();
            continue;
        }

        exec_command(&cmd);
    }

    write_str(STDOUT_FILENO, "Encerrando o minishell.\n");
    return 0;
}
