#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"
#include "executor.h"
#include "input.h"
#include "output.h"
#include "parser.h"
#include "trace.h"

static void report_parse_error(ParseStatus status)
{
    if (status == PARSE_TOO_MANY_ARGS)
        write_str(STDERR_FILENO, "minishell: argumentos demais\n");
    else if (status == PARSE_SYNTAX_ERROR)
        write_str(STDERR_FILENO, "minishell: erro de sintaxe\n");
    else if (status == PARSE_TOO_MANY_CMDS)
        write_str(STDERR_FILENO, "minishell: comandos demais no pipeline\n");
}

static int pipeline_has_builtin(const Pipeline *pipeline)
{
    for (int i = 0; i < pipeline->count; i++)
        if (is_builtin(pipeline->cmds[i].argv[0]))
            return 1;
    return 0;
}

int main(int argc, char **argv)
{
    char line[LINE_SIZE];
    Pipeline pipeline;
    int keep_running = 1;

    if (argc > 1 && strcmp(argv[1], "--trace") == 0)
        trace_enable(1);
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

        if (pipeline_has_builtin(&pipeline)) {
            if (pipeline.count == 1 && pipeline.cmds[0].out_file == NULL)
                builtin_run(&pipeline.cmds[0], &keep_running);
            else
                report_builtin_misuse();
            continue;
        }

        exec_pipeline(&pipeline);
    }

    write_str(STDOUT_FILENO, "Encerrando o minishell.\n");
    return 0;
}
