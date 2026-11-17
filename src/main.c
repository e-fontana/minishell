#include <signal.h>
#include <unistd.h>

#include "builtins.h"
#include "executor.h"
#include "input.h"
#include "output.h"
#include "parser.h"

static void report_parse_error(ParseStatus status)
{
    if (status == PARSE_TOO_MANY_ARGS)
        write_str(STDERR_FILENO, "minishell: argumentos demais\n");
    else if (status == PARSE_SYNTAX_ERROR)
        write_str(STDERR_FILENO, "minishell: erro de sintaxe\n");
    else if (status == PARSE_TOO_MANY_CMDS)
        write_str(STDERR_FILENO, "minishell: comandos demais no pipeline\n");
}

int main(void)
{
    char line[LINE_SIZE];
    Pipeline pipeline;
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

        ParseStatus parse_status = parse_line(line, &pipeline);
        if (parse_status != PARSE_OK) {
            report_parse_error(parse_status);
            continue;
        }

        if (pipeline.count == 1 && builtin_run(&pipeline.cmds[0], &keep_running))
            continue;

        exec_pipeline(&pipeline);
    }

    write_str(STDOUT_FILENO, "Encerrando o minishell.\n");
    return 0;
}
