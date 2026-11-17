# Mini Shell

Interpretador de comandos simples para Unix, escrito em C para o Projeto
Integrador de Sistemas Operacionais (UFBA). Ele lê uma linha, separa o comando
e seus argumentos e o executa em um processo filho, usando diretamente as
chamadas ao sistema (`read`, `fork`, `execvp`, `wait`, ...) em vez de
`system()` ou da entrada da stdio.

## Como compilar e rodar

Requisitos: Linux, `gcc` e `make`.
```bash
make            # compila os objetos em build/ e gera ./minishell
./minishell     # inicia o shell (prompt "> ")
make test       # testes do parser e, em seguida, cenários de ponta a ponta
make clean      # remove build/ e ./minishell
```

Para sair, digite `exit` ou pressione Ctrl+D (fim da entrada). Ctrl+C
interrompe o comando em execução, não o shell.

## Estrutura do código

| Arquivo | Responsabilidade |
|---------|------------------|
| `src/main.c` | Laço principal: prompt, leitura, análise, despacho para um comando interno ou para o executor |
| `src/io/input.c` | `read_line()`: lê uma linha da entrada padrão, um byte por `read()` |
| `src/parser/parser.c` | `parse_line()`: separa a linha em palavras e no arquivo após `>`; não faz chamadas ao sistema |
| `src/exec/executor.c` | `exec_command()`: cria o filho, redireciona a saída, executa o programa e espera seu término |
| `src/exec/builtins.c` | Tabela de comandos internos (`cd`, `exit`) e a mensagem de uso indevido |
| `src/io/output.c` | `write_str()`: escreve um texto em um descritor com `write()` |
| `include/parser/parser.h` | Tipos `Command` e `ParseStatus`, limite `MAX_ARGS` e `parse_line()` |
| `include/io/input.h` | Tipo `ReadStatus`, tamanho `LINE_SIZE` e `read_line()` |
| `include/exec/executor.h` | `exec_command()` |
| `include/exec/builtins.h` | `is_builtin()`, `builtin_run()` e `report_builtin_misuse()` |
| `include/io/output.h` | `write_str()` |

Fluxo de dados:

1. `main` escreve o prompt `> ` e chama `read_line`, que lê até o `\n`.
2. `parse_line` divide a linha em palavras (espaço ou tab) e guarda o arquivo após `>`.
3. Se a primeira palavra é `cd` ou `exit`, o próprio shell executa o comando.
4. Caso contrário, `exec_command` faz `fork`; o filho abre o arquivo de saída (se houver) e chama `execvp`.
5. O pai espera o filho com `wait` e volta ao passo 1.

## Chamadas ao sistema utilizadas

| Chamada | Módulo | Para quê |
|---------|--------|----------|
| `read()` | `src/io/input.c` | Ler a linha de comando do descritor 0 |
| `write()` | `src/io/output.c` | Escrever o prompt e as mensagens |
| `fork()` | `src/exec/executor.c` | Criar o processo filho do comando |
| `execvp()` | `src/exec/executor.c` | Substituir o filho pelo programa, buscando no `PATH` |
| `wait()` | `src/exec/executor.c` | Esperar o filho e obter seu estado de saída |
| `open()` | `src/exec/executor.c` | Abrir o arquivo após `>` (`O_WRONLY \| O_CREAT \| O_TRUNC`) |
| `dup2()` | `src/exec/executor.c` | Ligar o arquivo aberto à saída padrão (descritor 1) |
| `close()` | `src/exec/executor.c` | Fechar o descritor original do arquivo após o `dup2` |
| `chdir()` | `src/exec/builtins.c` | Comando interno `cd` |
| `signal()` | `src/main.c`, `src/exec/executor.c` | O shell ignora SIGINT; o filho restaura o padrão |
| `_exit()` | `src/exec/executor.c` | Encerrar o filho quando `open`, `dup2` ou `execvp` falha |
| `strerror()` / `perror()` (funções da biblioteca C, não chamadas ao sistema) | `src/exec/executor.c` / `src/exec/builtins.c` | Texto do `errno` nas mensagens de erro |

## Exemplos de comandos testados

Sessão real, em um diretório com `notas.txt` (três linhas) e `pasta/dentro.txt`:
```bash
printf 'echo Ola mundo\nls\ncat -n notas.txt\ncomandoquenaoexiste\ncd pasta\nls\ncd /nao/existe\ncd ..\necho oi > arquivo\ncat arquivo\nexit\n' | ./minishell
```

Saída obtida. Como a entrada vem de um pipe, os comandos não aparecem; cada
`> ` é um prompt, e `cd pasta`, `cd ..` e `echo oi > arquivo` não imprimem nada:
```
> Ola mundo
> notas.txt
pasta
>      1	linha um
     2	linha dois
     3	linha tres
> minishell: comando nao encontrado: comandoquenaoexiste
> > dentro.txt
> minishell: cd: No such file or directory
> > > oi
> Encerrando o minishell.
```

## Limitações conhecidas

- Sem pipes (`|`), sem `<`, sem `>>` e sem execução em segundo plano (`&`).
- Sem aspas nem escapes: `echo "a b"` passa as aspas como parte das palavras.
- Sem expansão de `$VAR`, `~` ou curingas (`*`, `?`).
- Um comando por linha (sem `;`, `&&` ou `||`).
- `cd` sem argumento é erro; não vai para o `HOME`.
- Comandos internos (`cd`, `exit`) não aceitam `>`.
- O `>` precisa de espaços (`a>b` é uma palavra comum). O `|` é sempre uma palavra comum: `ls | wc` passa `|` e `wc` como argumentos de `ls`.
- Limites: 1023 caracteres por linha e 63 palavras por comando; acima disso a linha é ignorada com um aviso.
- Sem histórico e sem edição de linha.
- A entrada é lida um byte por `read()`: simples, porém lento.

## Testes

`make test` roda `tests/test_parser.c` (testes unitários de `parse_line()`: linhas
vazias, separadores, limite de palavras, `>` e erros de sintaxe) e depois
`tests/scenarios.sh` (executa o `./minishell` com entradas prontas e compara a saída).
