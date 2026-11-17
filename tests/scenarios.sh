#!/bin/sh

LC_ALL=C
export LC_ALL

SHELL_BIN=./minishell
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
passed=0
failed=0

run_shell() {
    printf "$1" | timeout 10 "$SHELL_BIN" 2>&1 | head -c 100000
    printf x
}

record() {
    if [ "$2" = ok ]; then
        passed=$((passed + 1))
    else
        failed=$((failed + 1))
        echo "FAIL $1"
    fi
}

check() {
    actual=$(run_shell "$2")
    expected=$(printf "$3"; printf x)
    if [ "$actual" = "$expected" ]; then
        record "$1" ok
    else
        record "$1" fail
    fi
}

check_contains() {
    actual=$(run_shell "$2")
    case "$actual" in
        *"$3"*) record "$1" ok ;;
        *) record "$1" fail ;;
    esac
}

check_absent() {
    if [ -e "$2" ]; then
        record "$1" fail
    else
        record "$1" ok
    fi
}

repeat() {
    count=0
    result=
    while [ "$count" -lt "$2" ]; do
        result="$result$1"
        count=$((count + 1))
    done
    printf '%s' "$result"
}

long_line=$(repeat a 1100)
many_args="echo$(repeat ' x' 70)"
nine_stages="echo a$(repeat ' | echo a' 8)"
builtin_message='minishell: comando interno nao pode ser usado em pipeline ou com redirecionamento'

check echo_then_exit 'echo hi\nexit\n' '> hi\n> Encerrando o minishell.\n'
check eof_after_command 'echo hi\n' '> hi\n> \nEncerrando o minishell.\n'
check eof_without_newline 'echo oi' '> oi\n> \nEncerrando o minishell.\n'
check blank_lines '\n\nexit\n' '> > > Encerrando o minishell.\n'
check command_not_found 'naoexiste\nexit\n' '> minishell: comando nao encontrado: naoexiste\n> Encerrando o minishell.\n'
check cd_without_argument 'cd\nexit\n' '> minishell: cd: informe o diretorio\n> Encerrando o minishell.\n'
check cd_missing_dir 'cd /nao/existe\nexit\n' '> minishell: cd: No such file or directory\n> Encerrando o minishell.\n'
check cd_then_pwd 'cd /tmp\npwd\nexit\n' '> > /tmp\n> Encerrando o minishell.\n'
check line_too_long "${long_line}\\nexit\\n" '> minishell: linha muito longa\n> Encerrando o minishell.\n'
check too_many_args "${many_args}\\nexit\\n" '> minishell: argumentos demais\n> Encerrando o minishell.\n'
check echo_args 'echo a b c\nexit\n' '> a b c\n> Encerrando o minishell.\n'
check_contains cat_file 'cat Makefile\nexit\n' 'CFLAGS ='
check redirect_writes_file "echo hi > $work/ms_out.txt\\ncat $work/ms_out.txt\\nexit\\n" '> > hi\n> Encerrando o minishell.\n'
check redirect_open_fails 'echo hi > /nao/existe/f\nexit\n' '> minishell: nao foi possivel abrir /nao/existe/f: No such file or directory\n> Encerrando o minishell.\n'
check redirect_syntax_error 'echo hi >\nexit\n' '> minishell: erro de sintaxe\n> Encerrando o minishell.\n'
check builtin_with_redirect "cd /tmp > $work/ms_builtin_redirect.txt\\nexit\\n" "> ${builtin_message}\\n> Encerrando o minishell.\\n"
check_absent builtin_with_redirect_creates_no_file "$work/ms_builtin_redirect.txt"
check pipe_basic 'echo a b c | wc -w\nexit\n' '> 3\n> Encerrando o minishell.\n'
check pipe_three_stages 'echo hi | cat | cat\nexit\n' '> hi\n> Encerrando o minishell.\n'
check pipe_does_not_hang 'yes | head -1\nexit\n' '> y\n> Encerrando o minishell.\n'
check pipe_into_file "echo hi | cat > $work/ms_p.txt\\ncat $work/ms_p.txt\\nexit\\n" '> > hi\n> Encerrando o minishell.\n'
check pipe_syntax_error 'ls |\nexit\n' '> minishell: erro de sintaxe\n> Encerrando o minishell.\n'
check pipe_too_many_commands "${nine_stages}\\nexit\\n" '> minishell: comandos demais no pipeline\n> Encerrando o minishell.\n'
check builtin_in_pipeline 'cd /tmp | cat\nexit\n' "> ${builtin_message}\\n> Encerrando o minishell.\\n"
check_contains pipe_missing_command_keeps_going 'naoexiste | echo ok\nexit\n' 'ok'

echo "$passed passed, $failed failed"
[ "$failed" -eq 0 ]
