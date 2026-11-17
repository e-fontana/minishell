#include <stddef.h>

#include "parser.h"

static int is_separator(char c)
{
    return c == ' ' || c == '\t';
}

int split_words(char *line, char **argv, int max_args)
{
    int count = 0;
    char *cursor = line;

    while (*cursor != '\0') {
        while (is_separator(*cursor))
            *cursor++ = '\0';
        if (*cursor == '\0')
            break;

        if (count >= max_args - 1)
            return -1;
        argv[count++] = cursor;

        while (*cursor != '\0' && !is_separator(*cursor))
            cursor++;
    }

    argv[count] = NULL;
    return count;
}
