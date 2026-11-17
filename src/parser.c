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

    if (cmd->argc == 0 || cmd->out_file != NULL || file == NULL || is_redirect(file))
        return PARSE_SYNTAX_ERROR;
    cmd->out_file = file;
    return PARSE_OK;
}

ParseStatus parse_line(char *line, Pipeline *out)
{
    char *cursor = line;
    char *token;
    Command *current = &out->cmds[0];

    out->count = 0;
    start_command(current);

    while ((token = next_token(&cursor)) != NULL) {
        ParseStatus status = is_redirect(token)
            ? set_out_file(current, &cursor)
            : add_word(current, token);
        if (status != PARSE_OK)
            return status;
    }

    if (current->argc == 0)
        return PARSE_EMPTY;

    out->count = 1;
    return PARSE_OK;
}
