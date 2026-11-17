#ifndef PARSER_H
#define PARSER_H

#define MAX_ARGS 64

typedef struct {
    char *argv[MAX_ARGS];
    int argc;
    char *out_file;
} Command;

typedef enum {
    PARSE_OK,
    PARSE_EMPTY,
    PARSE_TOO_MANY_ARGS,
    PARSE_SYNTAX_ERROR
} ParseStatus;

ParseStatus parse_line(char *line, Command *out);

#endif
