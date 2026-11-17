# Mini Shell

A small Unix command interpreter written in C for the Operating Systems course
project (Projeto Integrador de Sistemas Operacionais, UFBA). It reads commands
with `read()`, starts processes with `fork()` and `execvp()`, connects them
with `pipe()` and `dup2()`, and waits for them with `waitpid()`, calling the
system directly instead of relying on `system()` or stdio for input. Messages
printed to the user are in Portuguese without accents.

## Build, run, test

Requirements: Linux, `gcc` and `make`.

```bash
make                    # objects in build/, binary ./minishell
./minishell             # start the shell; leave with exit or Ctrl+D
./minishell --trace     # also print system calls to stderr
make test               # parser unit tests, then end-to-end scenarios
make clean              # remove build/ and ./minishell
```

## Supported syntax

- A simple command with arguments: `ls -l /tmp`.
- Output redirection to a file (created or truncated): `echo hi > out.txt`.
  Only on the last command of a line.
- Pipes between up to 8 commands: `ls | grep c | wc -l`.
- Builtins `cd DIR` and `exit`, run by the shell itself.
- Tokens are separated by spaces or tabs, so `ls|wc` and `a>b` are ordinary
  words, not operators.
- Malformed `>` or `|` usage (`echo >`, `| ls`, a second `>`, `>` on a
  non-final command) prints `minishell: erro de sintaxe` and returns to the prompt.
- Ctrl+C stops the running command, not the shell.

## Modules

| File              | Responsibility                                                          |
|-------------------|-------------------------------------------------------------------------|
| `src/main.c`      | Main loop: prompt, read, parse, dispatch to a builtin or the executor, refuse builtins in a pipeline or with `>`; `--trace` flag |
| `src/input.c`     | `read_line()` reads one line from stdin, one byte per `read()`           |
| `src/parser.c`    | `parse_line()` splits a line into a `Pipeline` of `Command`s; no system calls |
| `src/executor.c`  | `exec_pipeline()` creates pipes, forks each stage, redirects, execs and waits |
| `src/builtins.c`  | Table of builtins (`cd`, `exit`) and the misuse message                  |
| `src/output.c`    | `write_str()` writes a string to a descriptor with `write()`             |
| `src/trace.c`     | `trace()` prints `[trace]` lines to stderr when `--trace` is on          |

Each module except `main.c` has a matching `include/X.h` exposing only what
other modules use.

## System calls

| Call        | Module                              | Purpose                                              |
|-------------|-------------------------------------|------------------------------------------------------|
| `read()`    | `src/input.c`                       | Read the command line from descriptor 0              |
| `write()`   | `src/output.c`, `src/trace.c`       | Prompt, messages and trace lines                     |
| `fork()`    | `src/executor.c`                    | Create one child per command                         |
| `execvp()`  | `src/executor.c`                    | Replace the child with the program, searching `PATH` |
| `waitpid()` | `src/executor.c`                    | Wait for each child and decode its status            |
| `pipe()`    | `src/executor.c`                    | Connect one command's output to the next one's input |
| `dup2()`    | `src/executor.c`                    | Move pipe ends and the output file onto fds 0 and 1  |
| `open()`    | `src/executor.c`                    | Open the file after `>` (`O_WRONLY O_CREAT O_TRUNC`) |
| `close()`   | `src/executor.c`                    | Close unused pipe ends in parent and children        |
| `chdir()`   | `src/builtins.c`                    | Builtin `cd`                                         |
| `signal()`  | `src/main.c`, `src/executor.c`      | Shell ignores SIGINT; children restore the default   |
| `_exit()`   | `src/executor.c`                    | End a child when `open`, `dup2` or `execvp` fails    |
| `strerror()` | `src/executor.c`                   | Text of `errno` in `open`, `exec`, `dup2`, `pipe`, `fork` and `wait` errors |
| `perror()`  | `src/builtins.c`                    | Error message of the builtin `cd`                    |

Exit statuses of children: 127 command not found, 126 other exec failure,
1 when the `>` file cannot be opened, 128+n when killed by signal n.

## Tracing

With `--trace` (the first argument), the shell writes one line to stderr for
each `pipe`, `fork`, `dup2`, `execvp` and `waitpid`. Parent and child lines may
interleave in any order. Captured run:

```
> echo a b c | wc -w > contagem.txt
[trace] pipe -> 3 4
[trace] fork -> child 177259
[trace] dup2(4, 1)
[trace] execvp echo
[trace] fork -> child 177260
[trace] dup2(3, 0)
[trace] dup2(3, 1)
[trace] execvp wc
[trace] wait 177259 -> status 0
[trace] wait 177260 -> status 0
> cat contagem.txt
[trace] fork -> child 177275
[trace] execvp cat
3
[trace] wait 177275 -> status 0
> exit
Encerrando o minishell.
```

## Example session

```
> echo Ola mundo
Ola mundo
> ls
Makefile  include  notes.txt  src  tests
> echo primeira linha > saida.txt
> cat saida.txt
primeira linha
> ls | wc -l
6
> echo a b c | wc -w
3
> comandoquenaoexiste
minishell: comando nao encontrado: comandoquenaoexiste
> cd /nao/existe
minishell: cd: No such file or directory
> exit
Encerrando o minishell.
```

## Limits

1023 characters per line, 63 words per command, 8 commands per pipeline. Over
a limit the shell prints a warning, ignores the line and keeps running.

## Known limitations

- No quotes or escapes: `echo "a   b"` splits on spaces and passes the quotes.
- No expansion of `$VAR`, `~` or wildcards.
- No `<`, no `>>`, no background `&`, no `;`, `&&` or `||`.
- Builtins cannot be used inside a pipeline or with `>`.
- `cd` needs a directory; it does not default to `HOME`.
- No history and no line editing.
- Input is read one byte per `read()`: simple, but slow.
