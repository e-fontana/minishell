#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "parser.h"

static char buffer[256];
static Command cmd;

static ParseStatus parse(const char *text)
{
    strcpy(buffer, text);
    return parse_line(buffer, &cmd);
}

static void expect_words(const char *text, int argc, const char *const *words)
{
    assert(parse(text) == PARSE_OK);
    assert(cmd.argc == argc);
    for (int i = 0; i < argc; i++)
        assert(strcmp(cmd.argv[i], words[i]) == 0);
    assert(cmd.argv[argc] == NULL);
    assert(cmd.out_file == NULL);
}

static void test_empty_lines(void)
{
    assert(parse("") == PARSE_EMPTY);
    assert(parse("  \t ") == PARSE_EMPTY);
}

static void test_plain_words(void)
{
    const char *ls[] = { "ls" };
    const char *ls_l[] = { "ls", "-l" };
    const char *ls_tmp[] = { "ls", "-l", "/tmp" };
    const char *echo_hi[] = { "echo", "hi" };

    expect_words("ls", 1, ls);
    expect_words("ls -l /tmp", 3, ls_tmp);
    expect_words("echo\thi", 2, echo_hi);
    expect_words("  ls", 1, ls);
    expect_words("ls  ", 1, ls);
    expect_words("\t ls -l \t", 2, ls_l);
}

static void test_operators_need_spaces(void)
{
    const char *pipe_word[] = { "ls|wc" };
    const char *redirect_word[] = { "a>b" };
    const char *pipe_spaced[] = { "ls", "|", "wc" };

    expect_words("ls|wc", 1, pipe_word);
    expect_words("a>b", 1, redirect_word);
    expect_words("ls | wc", 3, pipe_spaced);
}

static void test_max_args_boundary(void)
{
    char line[256] = "";

    for (int i = 0; i < MAX_ARGS - 1; i++)
        strcat(line, i == 0 ? "w" : " w");
    assert(parse(line) == PARSE_OK);
    assert(cmd.argc == MAX_ARGS - 1);
    assert(strcmp(cmd.argv[MAX_ARGS - 2], "w") == 0);
    assert(cmd.argv[MAX_ARGS - 1] == NULL);

    strcat(line, " w");
    assert(parse(line) == PARSE_TOO_MANY_ARGS);
    assert(cmd.argc == MAX_ARGS - 1);
    assert(cmd.argv[MAX_ARGS - 1] == NULL);
    assert(cmd.out_file == NULL);
}

static void test_redirect(void)
{
    assert(parse("echo hi > out.txt") == PARSE_OK);
    assert(cmd.argc == 2);
    assert(strcmp(cmd.argv[0], "echo") == 0);
    assert(strcmp(cmd.argv[1], "hi") == 0);
    assert(cmd.argv[2] == NULL);
    assert(strcmp(cmd.out_file, "out.txt") == 0);

    assert(parse("echo x > f extra") == PARSE_OK);
    assert(cmd.argc == 3);
    assert(strcmp(cmd.argv[2], "extra") == 0);
    assert(cmd.argv[3] == NULL);
    assert(strcmp(cmd.out_file, "f") == 0);
}

static void test_redirect_syntax_errors(void)
{
    const char *lines[] = { "echo hi >", "echo a > b > c", "> out.txt", "echo > >", ">" };

    for (size_t i = 0; i < sizeof(lines) / sizeof(lines[0]); i++)
        assert(parse(lines[i]) == PARSE_SYNTAX_ERROR);
}

int main(void)
{
    test_empty_lines();
    test_plain_words();
    test_operators_need_spaces();
    test_max_args_boundary();
    test_redirect();
    test_redirect_syntax_errors();
    return 0;
}
