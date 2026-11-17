#include <stdarg.h>
#include <stdio.h>
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
    char line[TRACE_LINE_SIZE];
    va_list args;

    if (!trace_enabled)
        return;

    va_start(args, format);
    int length = vsnprintf(line, sizeof(line), format, args);
    va_end(args);

    if (length < 0)
        return;
    if (length > TRACE_LINE_SIZE - 1)
        length = TRACE_LINE_SIZE - 1;
    if (length == 0 || line[length - 1] != '\n') {
        if (length == TRACE_LINE_SIZE - 1)
            length--;
        line[length++] = '\n';
    }
    ssize_t ignored = write(STDERR_FILENO, line, (size_t)length);
    (void)ignored;
}
