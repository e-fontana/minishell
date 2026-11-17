#include <stddef.h>
#include <string.h>

#include "parser/parser.h"

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

ParseStatus parse_line(char *line, Command *out)
{
    char *cursor = line;
    char *token;

    out->argc = 0;
    out->argv[0] = NULL;
    out->out_file = NULL;

    while ((token = next_token(&cursor)) != NULL) {
        ParseStatus status;

        if (is_redirect(token))
            status = set_out_file(out, &cursor);
        else
            status = add_word(out, token);
        if (status != PARSE_OK)
            return status;
    }

    return out->argc == 0 ? PARSE_EMPTY : PARSE_OK;
}
