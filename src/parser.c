#include <stddef.h>
#include <string.h>

#include "parser.h"

static int is_separator(char c)
{
    return c == ' ' || c == '\t';
}

static char *next_token(char **cursor)
{
    while (is_separator(**cursor))
        *(*cursor)++ = '\0';
    if (**cursor == '\0')
        return NULL;

    char *token = *cursor;

    while (**cursor != '\0' && !is_separator(**cursor))
        (*cursor)++;
    if (**cursor != '\0')
        *(*cursor)++ = '\0';
    return token;
}

static int is_redirect(const char *token)
{
    return strcmp(token, ">") == 0;
}

static int is_pipe(const char *token)
{
    return strcmp(token, "|") == 0;
}

static int is_operator(const char *token)
{
    return is_redirect(token) || is_pipe(token);
}

static void start_command(Command *cmd)
{
    cmd->argc = 0;
    cmd->argv[0] = NULL;
    cmd->out_file = NULL;
}

static ParseStatus add_word(Command *cmd, char *word)
{
    if (cmd->argc >= MAX_ARGS - 1)
        return PARSE_TOO_MANY_ARGS;
    cmd->argv[cmd->argc++] = word;
    cmd->argv[cmd->argc] = NULL;
    return PARSE_OK;
}

static ParseStatus set_out_file(Command *cmd, char **cursor)
{
    char *file = next_token(cursor);

    if (cmd->argc == 0 || cmd->out_file != NULL || file == NULL || is_operator(file))
        return PARSE_SYNTAX_ERROR;
    cmd->out_file = file;
    return PARSE_OK;
}

static ParseStatus open_next_command(Pipeline *out, int *count)
{
    const Command *finished = &out->cmds[*count - 1];

    if (finished->argc == 0 || finished->out_file != NULL)
        return PARSE_SYNTAX_ERROR;
    if (*count == MAX_CMDS)
        return PARSE_TOO_MANY_CMDS;
    start_command(&out->cmds[(*count)++]);
    return PARSE_OK;
}

ParseStatus parse_line(char *line, Pipeline *out)
{
    char *cursor = line;
    char *token;
    int count = 1;

    out->count = 0;
    start_command(&out->cmds[0]);

    while ((token = next_token(&cursor)) != NULL) {
        Command *current = &out->cmds[count - 1];
        ParseStatus status;

        if (is_pipe(token))
            status = open_next_command(out, &count);
        else if (is_redirect(token))
            status = set_out_file(current, &cursor);
        else
            status = add_word(current, token);
        if (status != PARSE_OK)
            return status;
    }

    if (out->cmds[count - 1].argc == 0)
        return count == 1 ? PARSE_EMPTY : PARSE_SYNTAX_ERROR;

    out->count = count;
    return PARSE_OK;
}
