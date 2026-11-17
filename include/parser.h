#ifndef PARSER_H
#define PARSER_H

#define MAX_ARGS 64
#define MAX_CMDS 8

typedef struct {
    char *argv[MAX_ARGS];
    int argc;
    char *out_file;
} Command;

typedef struct {
    Command cmds[MAX_CMDS];
    int count;
} Pipeline;

typedef enum {
    PARSE_OK,
    PARSE_EMPTY,
    PARSE_TOO_MANY_ARGS,
    PARSE_TOO_MANY_CMDS,
    PARSE_SYNTAX_ERROR
} ParseStatus;

ParseStatus parse_line(char *line, Pipeline *out);

#endif
