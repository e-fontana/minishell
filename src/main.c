#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "minishell.h"

/*
 * Comandos internos: precisam rodar no proprio shell, nao em um filho.
 * (um "cd" executado no filho mudaria so o diretorio do filho.)
 *
 * Retorna 1 se o comando era interno e ja foi tratado, 0 caso contrario.
 */
static int comando_interno(char **args, int *continuar)
{
    if (strcmp(args[0], "exit") == 0) {
        *continuar = 0;
        return 1;
    }

    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL)
            escrever(STDERR_FILENO, "minishell: cd: informe o diretorio\n");
        else if (chdir(args[1]) < 0)
            perror("minishell: cd");
        return 1;
    }

    return 0;
}

/* Laco principal: prompt -> leitura -> parsing -> execucao. */
int main(void)
{
    char linha[TAM_LINHA];
    char *args[MAX_ARGS];
    int continuar = 1;

    /* o shell ignora Ctrl+C; quem recebe o sinal e o comando em execucao */
    signal(SIGINT, SIG_IGN);

    while (continuar) {
        escrever(STDOUT_FILENO, "> ");

        int lido = ler_linha(linha, TAM_LINHA);
        if (lido == 0) { /* Ctrl+D */
            escrever(STDOUT_FILENO, "\n");
            break;
        }
        if (lido < 0) {
            escrever(STDERR_FILENO, "minishell: linha muito longa\n");
            continue;
        }

        int n = separar_argumentos(linha, args, MAX_ARGS);
        if (n < 0) {
            escrever(STDERR_FILENO, "minishell: argumentos demais\n");
            continue;
        }
        if (n == 0) /* linha vazia */
            continue;

        if (comando_interno(args, &continuar))
            continue;

        executar_comando(args);
    }

    escrever(STDOUT_FILENO, "Encerrando o minishell.\n");
    return 0;
}
