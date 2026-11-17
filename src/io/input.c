#include <unistd.h>

#include "io/input.h"

ReadStatus read_line(char *line, int size)
{
    int length = 0;
    int overflowed = 0;
    char c;

    for (;;) {
        ssize_t count = read(STDIN_FILENO, &c, 1);

        if (count < 0)
            return READ_EOF;
        if (count == 0) {
            if (length == 0 && !overflowed)
                return READ_EOF;
            break;
        }
        if (c == '\n')
            break;

        if (length < size - 1)
            line[length++] = c;
        else
            overflowed = 1;
    }

    line[length] = '\0';
    return overflowed ? READ_TOO_LONG : READ_OK;
}
