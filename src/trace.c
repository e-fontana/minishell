#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "trace.h"

#define TRACE_LINE_SIZE 256

static int trace_enabled = 0;

void trace_enable(int enabled)
{
    trace_enabled = enabled;
}

void trace(const char *format, ...)
{
    char line[TRACE_LINE_SIZE] = "[trace] ";
    size_t length = strlen(line);
    va_list args;

    if (!trace_enabled)
        return;

    va_start(args, format);
    int written = vsnprintf(line + length, sizeof(line) - length, format, args);
    va_end(args);
    if (written < 0)
        return;

    length += (size_t)written;
    if (length > sizeof(line) - 2)
        length = sizeof(line) - 2;
    line[length++] = '\n';
    ssize_t ignored = write(STDERR_FILENO, line, length);
    (void)ignored;
}
