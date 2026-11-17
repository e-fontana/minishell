#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "minishell.h"

/*
 * Executa um comando externo:
 *   1. fork()   cria o processo filho;
 *   2. execvp() substitui o filho pelo programa pedido (procurando no PATH);
 *   3. wait()   faz o pai aguardar o filho terminar.
 */
void executar_comando(char **args)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("minishell: erro no fork");
        return;
    }

    if (pid == 0) {
        /* ---- processo filho ---- */
        signal(SIGINT, SIG_DFL); /* Ctrl+C volta a encerrar o comando */

        execvp(args[0], args);

        /* so chega aqui se o execvp falhou */
        if (errno == ENOENT) {
            escrever(STDERR_FILENO, "minishell: comando nao encontrado: ");
            escrever(STDERR_FILENO, args[0]);
            escrever(STDERR_FILENO, "\n");
        } else {
            escrever(STDERR_FILENO, "minishell: nao foi possivel executar ");
            escrever(STDERR_FILENO, args[0]);
            escrever(STDERR_FILENO, ": ");
            escrever(STDERR_FILENO, strerror(errno));
            escrever(STDERR_FILENO, "\n");
        }
        _exit(127);
    }

    /* ---- processo pai ---- */
    int status;

    if (wait(&status) < 0) {
        perror("minishell: erro no wait");
        return;
    }

    if (WIFSIGNALED(status))
        escrever(STDERR_FILENO, "\nminishell: comando interrompido por sinal\n");
}
