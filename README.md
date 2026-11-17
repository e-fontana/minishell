# Mini Shell

Projeto Integrador de Sistemas Operacionais (UFBA): um interpretador de
comandos simples, escrito em C, que usa diretamente as chamadas ao sistema do
Linux para ler comandos, criar processos e executar programas.

## Como compilar e rodar

Requisitos: Linux, `gcc` e `make`.

```bash
make
./minishell
```

Sem `make`:

```bash
gcc -Wall -Wextra -Iinclude -o minishell src/*.c
```

`make clean` apaga o executável e a pasta `build/`.

Para sair do shell, digite `exit` (ou pressione Ctrl+D).

## Estrutura do código

```
minishell/
├── Makefile
├── README.md
├── include/
│   └── minishell.h    constantes e protótipos
├── src/
│   ├── main.c         laço principal e comandos internos
│   ├── leitura.c      leitura e parsing
│   ├── execucao.c     criação de processo e execução
│   └── saida.c        escrita na tela
└── build/             arquivos .o (gerada pelo make)
```

| Arquivo                | Responsabilidade                                                    |
|------------------------|---------------------------------------------------------------------|
| `src/main.c`           | Laço principal (prompt, leitura, parsing, execução) e comandos internos `exit` e `cd` |
| `src/leitura.c`        | `ler_linha()` lê a entrada com `read()`; `separar_argumentos()` faz o parsing |
| `src/execucao.c`       | `executar_comando()` faz `fork()`, `execvp()` e `wait()`             |
| `src/saida.c`          | `escrever()` envia texto para a tela com `write()`                   |
| `include/minishell.h`  | Constantes e protótipos                                              |

Fluxo de cada comando:

1. O shell escreve o prompt `> `.
2. Lê uma linha da entrada padrão.
3. Separa a linha em palavras (`ls -l` vira `{"ls", "-l", NULL}`).
4. Se for `exit` ou `cd`, trata no próprio shell.
5. Caso contrário, cria um filho com `fork()`; o filho chama `execvp()` e o pai
   fica bloqueado em `wait()` até o filho terminar.

## Chamadas ao sistema utilizadas

| Chamada    | Onde                           | Para quê                                                        |
|------------|--------------------------------|-----------------------------------------------------------------|
| `read()`   | `src/leitura.c`                | Ler o comando da entrada padrão (descritor 0)                    |
| `write()`  | `src/saida.c`                  | Escrever o prompt e as mensagens (descritores 1 e 2)             |
| `fork()`   | `src/execucao.c`               | Criar o processo filho                                           |
| `execvp()` | `src/execucao.c`               | Substituir o filho pelo programa pedido, procurando no `PATH`    |
| `wait()`   | `src/execucao.c`               | Fazer o pai aguardar o término do filho                          |
| `chdir()`  | `src/main.c`                   | Implementar o comando interno `cd`                               |
| `signal()` | `src/main.c`, `src/execucao.c` | Ctrl+C encerra só o comando em execução, não o shell             |
| `_exit()`  | `src/execucao.c`               | Encerrar o filho quando o `execvp()` falha                       |

`perror()` e `strerror()` são usadas para transformar o código de erro
(`errno`) em mensagem.

## Exemplos de comandos testados

```
> echo Ola mundo
Ola mundo
> ls -l
total 44
drwxr-xr-x 2 fontana fontana  4096 16 nov 16:36 build
drwxr-xr-x 2 fontana fontana  4096 16 nov 16:36 include
-rw-r--r-- 1 fontana fontana   332 16 nov 16:36 Makefile
-rwxr-xr-x 1 fontana fontana 16944 16 nov 16:36 minishell
-rw-r--r-- 1 fontana fontana  4436 16 nov 16:36 README.md
drwxr-xr-x 2 fontana fontana  4096 16 nov 16:36 src
> cat include/minishell.h
#ifndef MINISHELL_H
#define MINISHELL_H
...
> comandoquenaoexiste
minishell: comando nao encontrado: comandoquenaoexiste
> ls /pasta/inexistente
ls: cannot access '/pasta/inexistente': No such file or directory
> cd /tmp
> pwd
/tmp
> cd /nao/existe
minishell: cd: No such file or directory
> exit
Encerrando o minishell.
```

## Limitações conhecidas

- Não há pipes (`|`), redirecionamento (`>`, `<`) nem execução em segundo
  plano (`&`).
- Não há aspas nem caracteres de escape: `echo "a   b"` é separado nos espaços
  e as aspas são passadas literalmente ao comando.
- Não há expansão de variáveis (`$HOME`), de `~` nem de curingas (`*.c`).
- Só um comando por linha (sem `;`, `&&`, `||`).
- Linha limitada a 1023 caracteres e 63 argumentos; acima disso o shell avisa e
  ignora a linha.
- `cd` exige o diretório (sem argumento não vai para o `HOME`).
- Sem histórico nem edição de linha com as setas.
- A leitura é feita um byte por `read()`, o que é simples mas pouco eficiente.
