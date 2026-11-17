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
    Command cmd;

    assert(parse_line(line, &cmd) == PARSE_EMPTY);
}

static void test_blank_line(void)
{
    char line[] = "  \t ";
    Command cmd;

    assert(parse_line(line, &cmd) == PARSE_EMPTY);
}

static void test_single_word(void)
{
    char line[] = "ls";
    Command cmd;

    assert(parse_line(line, &cmd) == PARSE_OK);
    assert(cmd.argc == 1);
    assert(strcmp(cmd.argv[0], "ls") == 0);
    assert(cmd.argv[1] == NULL);
}

static void test_words_with_args(void)
{
    char line[] = "ls -l /tmp";
    Command cmd;

    assert(parse_line(line, &cmd) == PARSE_OK);
    assert(cmd.argc == 3);
    assert(strcmp(cmd.argv[2], "/tmp") == 0);
    assert(cmd.argv[3] == NULL);
}

static void test_tab_separator(void)
{
    char line[] = "echo\thi";
    Command cmd;

    assert(parse_line(line, &cmd) == PARSE_OK);
    assert(cmd.argc == 2);
}

static void test_max_args_boundary(void)
{
    char line[256];
    Command cmd;

    build_words(line, MAX_ARGS - 1);
    assert(parse_line(line, &cmd) == PARSE_OK);
    assert(cmd.argc == MAX_ARGS - 1);
    assert(cmd.argv[MAX_ARGS - 1] == NULL);

    build_words(line, MAX_ARGS);
    assert(parse_line(line, &cmd) == PARSE_TOO_MANY_ARGS);
}

static void test_leading_and_trailing_whitespace(void)
{
    char leading[] = "  ls";
    char trailing[] = "ls  ";
    char mixed[] = "\t ls -l \t";
    Command cmd;

    assert(parse_line(leading, &cmd) == PARSE_OK);
    assert(cmd.argc == 1);
    assert(strcmp(cmd.argv[0], "ls") == 0);

    assert(parse_line(trailing, &cmd) == PARSE_OK);
    assert(cmd.argc == 1);
    assert(strcmp(cmd.argv[0], "ls") == 0);

    assert(parse_line(mixed, &cmd) == PARSE_OK);
    assert(cmd.argc == 2);
    assert(strcmp(cmd.argv[0], "ls") == 0);
    assert(strcmp(cmd.argv[1], "-l") == 0);
    assert(cmd.argv[2] == NULL);
}

static void test_redirect(void)
{
    char line[] = "echo hi > out.txt";
    Command cmd;

    assert(parse_line(line, &cmd) == PARSE_OK);
    assert(cmd.argc == 2);
    assert(strcmp(cmd.argv[0], "echo") == 0);
    assert(strcmp(cmd.argv[1], "hi") == 0);
    assert(cmd.argv[2] == NULL);
    assert(cmd.out_file != NULL);
    assert(strcmp(cmd.out_file, "out.txt") == 0);
}

static void test_no_redirect_leaves_out_file_null(void)
{
    char line[] = "echo hi";
    Command cmd;

    assert(parse_line(line, &cmd) == PARSE_OK);
    assert(cmd.out_file == NULL);
}

static void test_redirect_missing_file(void)
{
    char line[] = "echo hi >";
    Command cmd;

    assert(parse_line(line, &cmd) == PARSE_SYNTAX_ERROR);
}

static void test_redirect_twice(void)
{
    char line[] = "echo a > b > c";
    Command cmd;

    assert(parse_line(line, &cmd) == PARSE_SYNTAX_ERROR);
}

static void test_redirect_without_command(void)
{
    char line[] = "> out.txt";
    Command cmd;

    assert(parse_line(line, &cmd) == PARSE_SYNTAX_ERROR);
}

static void test_no_spaces_are_one_word(void)
{
    char pipe_line[] = "ls|wc";
    char redirect_line[] = "a>b";
    Command cmd;

    assert(parse_line(pipe_line, &cmd) == PARSE_OK);
    assert(cmd.argc == 1);
    assert(strcmp(cmd.argv[0], "ls|wc") == 0);

    assert(parse_line(redirect_line, &cmd) == PARSE_OK);
    assert(cmd.argc == 1);
    assert(strcmp(cmd.argv[0], "a>b") == 0);
    assert(cmd.out_file == NULL);
}

static void test_redirect_after_redirect_token(void)
{
    char line[] = "echo > >";
    Command cmd;

    assert(parse_line(line, &cmd) == PARSE_SYNTAX_ERROR);
}

static void test_redirect_alone(void)
{
    char line[] = ">";
    Command cmd;

    assert(parse_line(line, &cmd) == PARSE_SYNTAX_ERROR);
}

static void test_redirect_keeps_trailing_words(void)
{
    char line[] = "echo x > f extra";
    Command cmd;

    assert(parse_line(line, &cmd) == PARSE_OK);
    assert(cmd.argc == 3);
    assert(strcmp(cmd.argv[0], "echo") == 0);
    assert(strcmp(cmd.argv[1], "x") == 0);
    assert(strcmp(cmd.argv[2], "extra") == 0);
    assert(cmd.argv[3] == NULL);
    assert(strcmp(cmd.out_file, "f") == 0);
}

static void test_pipe_is_a_word(void)
{
    char line[] = "ls | wc";
    Command cmd;

    assert(parse_line(line, &cmd) == PARSE_OK);
    assert(cmd.argc == 3);
    assert(strcmp(cmd.argv[0], "ls") == 0);
    assert(strcmp(cmd.argv[1], "|") == 0);
    assert(strcmp(cmd.argv[2], "wc") == 0);
    assert(cmd.argv[3] == NULL);
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
    test_pipe_is_a_word();
    return 0;
}
