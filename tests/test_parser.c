#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "parser.h"

static void build_words(char *line, int word_count)
{
    line[0] = '\0';
    for (int i = 0; i < word_count; i++)
        strcat(line, i == 0 ? "w" : " w");
}

static void test_empty_line(void)
{
    char line[] = "";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_EMPTY);
}

static void test_blank_line(void)
{
    char line[] = "  \t ";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_EMPTY);
}

static void test_single_word(void)
{
    char line[] = "ls";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_OK);
    assert(pipeline.count == 1);
    assert(pipeline.cmds[0].argc == 1);
    assert(strcmp(pipeline.cmds[0].argv[0], "ls") == 0);
    assert(pipeline.cmds[0].argv[1] == NULL);
}

static void test_words_with_args(void)
{
    char line[] = "ls -l /tmp";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_OK);
    assert(pipeline.cmds[0].argc == 3);
    assert(strcmp(pipeline.cmds[0].argv[2], "/tmp") == 0);
    assert(pipeline.cmds[0].argv[3] == NULL);
}

static void test_tab_separator(void)
{
    char line[] = "echo\thi";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_OK);
    assert(pipeline.cmds[0].argc == 2);
}

static void test_max_args_boundary(void)
{
    char line[256];
    Pipeline pipeline;

    build_words(line, MAX_ARGS - 1);
    assert(parse_line(line, &pipeline) == PARSE_OK);
    assert(pipeline.cmds[0].argc == MAX_ARGS - 1);

    build_words(line, MAX_ARGS);
    assert(parse_line(line, &pipeline) == PARSE_TOO_MANY_ARGS);
}

static void test_no_spaces_are_one_word(void)
{
    char pipe_line[] = "ls|wc";
    char redirect_line[] = "a>b";
    Pipeline pipeline;

    assert(parse_line(pipe_line, &pipeline) == PARSE_OK);
    assert(pipeline.cmds[0].argc == 1);
    assert(strcmp(pipeline.cmds[0].argv[0], "ls|wc") == 0);

    assert(parse_line(redirect_line, &pipeline) == PARSE_OK);
    assert(pipeline.cmds[0].argc == 1);
    assert(strcmp(pipeline.cmds[0].argv[0], "a>b") == 0);
}

int main(void)
{
    test_empty_line();
    test_blank_line();
    test_single_word();
    test_words_with_args();
    test_tab_separator();
    test_max_args_boundary();
    test_no_spaces_are_one_word();
    return 0;
}
