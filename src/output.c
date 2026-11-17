#include <string.h>
#include <unistd.h>

#include "output.h"

void write_str(int fd, const char *text)
{
    size_t remaining = strlen(text);

    while (remaining > 0) {
        ssize_t written = write(fd, text, remaining);
        if (written <= 0)
            return;
        text += written;
        remaining -= (size_t)written;
    }
}
