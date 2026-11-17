#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "parser/parser.h"

static void build_words(char *line, int word_count)
{
    line[0] = '\0';
    for (int i = 0; i < word_count; i++)
        strcat(line, i == 0 ? "w" : " w");
}

static void build_commands(char *line, int command_count)
{
    line[0] = '\0';
    for (int i = 0; i < command_count; i++)
        strcat(line, i == 0 ? "echo a" : " | echo a");
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
    assert(pipeline.cmds[0].argv[MAX_ARGS - 1] == NULL);

    build_words(line, MAX_ARGS);
    assert(parse_line(line, &pipeline) == PARSE_TOO_MANY_ARGS);
    assert(pipeline.count == 0);
}

static void test_leading_and_trailing_whitespace(void)
{
    char leading[] = "  ls";
    char trailing[] = "ls  ";
    char mixed[] = "\t ls -l \t";
    Pipeline pipeline;

    assert(parse_line(leading, &pipeline) == PARSE_OK);
    assert(pipeline.cmds[0].argc == 1);
    assert(strcmp(pipeline.cmds[0].argv[0], "ls") == 0);

    assert(parse_line(trailing, &pipeline) == PARSE_OK);
    assert(pipeline.cmds[0].argc == 1);
    assert(strcmp(pipeline.cmds[0].argv[0], "ls") == 0);

    assert(parse_line(mixed, &pipeline) == PARSE_OK);
    assert(pipeline.cmds[0].argc == 2);
    assert(strcmp(pipeline.cmds[0].argv[0], "ls") == 0);
    assert(strcmp(pipeline.cmds[0].argv[1], "-l") == 0);
    assert(pipeline.cmds[0].argv[2] == NULL);
}

static void test_redirect(void)
{
    char line[] = "echo hi > out.txt";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_OK);
    assert(pipeline.count == 1);
    assert(pipeline.cmds[0].argc == 2);
    assert(strcmp(pipeline.cmds[0].argv[0], "echo") == 0);
    assert(strcmp(pipeline.cmds[0].argv[1], "hi") == 0);
    assert(pipeline.cmds[0].argv[2] == NULL);
    assert(pipeline.cmds[0].out_file != NULL);
    assert(strcmp(pipeline.cmds[0].out_file, "out.txt") == 0);
}

static void test_no_redirect_leaves_out_file_null(void)
{
    char line[] = "echo hi";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_OK);
    assert(pipeline.cmds[0].out_file == NULL);
}

static void test_redirect_missing_file(void)
{
    char line[] = "echo hi >";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_SYNTAX_ERROR);
    assert(pipeline.count == 0);
}

static void test_redirect_twice(void)
{
    char line[] = "echo a > b > c";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_SYNTAX_ERROR);
    assert(pipeline.count == 0);
}

static void test_redirect_without_command(void)
{
    char line[] = "> out.txt";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_SYNTAX_ERROR);
    assert(pipeline.count == 0);
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
    assert(pipeline.cmds[0].out_file == NULL);
}

static void test_redirect_after_redirect_token(void)
{
    char line[] = "echo > >";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_SYNTAX_ERROR);
    assert(pipeline.count == 0);
}

static void test_redirect_alone(void)
{
    char line[] = ">";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_SYNTAX_ERROR);
    assert(pipeline.count == 0);
}

static void test_redirect_keeps_trailing_words(void)
{
    char line[] = "echo x > f extra";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_OK);
    assert(pipeline.count == 1);
    assert(pipeline.cmds[0].argc == 3);
    assert(strcmp(pipeline.cmds[0].argv[0], "echo") == 0);
    assert(strcmp(pipeline.cmds[0].argv[1], "x") == 0);
    assert(strcmp(pipeline.cmds[0].argv[2], "extra") == 0);
    assert(pipeline.cmds[0].argv[3] == NULL);
    assert(strcmp(pipeline.cmds[0].out_file, "f") == 0);
}

static void test_redirect_into_pipe_token(void)
{
    char line[] = "echo > | cat";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_SYNTAX_ERROR);
    assert(pipeline.count == 0);
}

static void test_pipe_two(void)
{
    char line[] = "ls | wc -l";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_OK);
    assert(pipeline.count == 2);
    assert(pipeline.cmds[0].argc == 1);
    assert(strcmp(pipeline.cmds[0].argv[0], "ls") == 0);
    assert(pipeline.cmds[0].argv[1] == NULL);
    assert(pipeline.cmds[0].out_file == NULL);
    assert(pipeline.cmds[1].argc == 2);
    assert(strcmp(pipeline.cmds[1].argv[0], "wc") == 0);
    assert(strcmp(pipeline.cmds[1].argv[1], "-l") == 0);
    assert(pipeline.cmds[1].argv[2] == NULL);
    assert(pipeline.cmds[1].out_file == NULL);
}

static void test_pipe_leading(void)
{
    char line[] = "| ls";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_SYNTAX_ERROR);
    assert(pipeline.count == 0);
}

static void test_pipe_trailing(void)
{
    char line[] = "ls |";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_SYNTAX_ERROR);
    assert(pipeline.count == 0);
}

static void test_pipe_double(void)
{
    char line[] = "ls | | wc";
    Pipeline pipeline;

    assert(parse_line(line, &pipeline) == PARSE_SYNTAX_ERROR);
    assert(pipeline.count == 0);
}

static void test_redirect_only_on_last_stage(void)
{
    char middle[] = "echo a > f | cat";
    char last[] = "echo a | cat > f";
    Pipeline pipeline;

    assert(parse_line(middle, &pipeline) == PARSE_SYNTAX_ERROR);
    assert(pipeline.count == 0);

    assert(parse_line(last, &pipeline) == PARSE_OK);
    assert(pipeline.count == 2);
    assert(pipeline.cmds[0].out_file == NULL);
    assert(pipeline.cmds[1].argc == 1);
    assert(strcmp(pipeline.cmds[1].argv[0], "cat") == 0);
    assert(pipeline.cmds[1].out_file != NULL);
    assert(strcmp(pipeline.cmds[1].out_file, "f") == 0);
}

static void test_max_cmds_boundary(void)
{
    char line[256];
    Pipeline pipeline;

    build_commands(line, MAX_CMDS);
    assert(parse_line(line, &pipeline) == PARSE_OK);
    assert(pipeline.count == MAX_CMDS);
    assert(strcmp(pipeline.cmds[MAX_CMDS - 1].argv[0], "echo") == 0);
    assert(pipeline.cmds[MAX_CMDS - 1].argc == 2);

    build_commands(line, MAX_CMDS + 1);
    assert(parse_line(line, &pipeline) == PARSE_TOO_MANY_CMDS);
    assert(pipeline.count == 0);
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
    test_leading_and_trailing_whitespace();
    test_redirect();
    test_no_redirect_leaves_out_file_null();
    test_redirect_missing_file();
    test_redirect_twice();
    test_redirect_without_command();
    test_redirect_after_redirect_token();
    test_redirect_alone();
    test_redirect_keeps_trailing_words();
    test_redirect_into_pipe_token();
    test_pipe_two();
    test_pipe_leading();
    test_pipe_trailing();
    test_pipe_double();
    test_redirect_only_on_last_stage();
    test_max_cmds_boundary();
    return 0;
}
