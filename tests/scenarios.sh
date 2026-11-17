#!/bin/sh

LC_ALL=C
export LC_ALL

SHELL_BIN=./minishell
passed=0
failed=0

run_shell() {
    printf "$1" | "$SHELL_BIN" 2>&1
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

echo "$passed passed, $failed failed"
[ "$failed" -eq 0 ]
