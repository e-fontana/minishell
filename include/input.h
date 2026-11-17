#ifndef INPUT_H
#define INPUT_H

#define LINE_SIZE 1024

typedef enum { READ_OK, READ_EOF, READ_TOO_LONG } ReadStatus;

ReadStatus read_line(char *line, int size);

#endif
