#ifndef TRACE_H
#define TRACE_H

void trace_enable(int enabled);
void trace(const char *format, ...) __attribute__((format(printf, 1, 2)));

#endif
