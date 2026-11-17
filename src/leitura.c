#include <stddef.h>
#include <unistd.h>

#include "minishell.h"

/*
 * Le uma linha da entrada padrao com a chamada ao sistema read(),
 * um byte por vez, ate encontrar '\n'.
 *
 * Retorno:
 *    1  -> linha lida (sem o '\n', terminada em '\0')
 *    0  -> fim da entrada (Ctrl+D) ou erro de leitura
 *   -1  -> linha maior que o buffer (o resto da linha e descartado)
 */
int ler_linha(char *linha, int tamanho)
{
    int pos = 0;
    int estourou = 0;
    char c;

    for (;;) {
        ssize_t lidos = read(STDIN_FILENO, &c, 1);

        if (lidos < 0)
            return 0;
        if (lidos == 0) {
            /* fim da entrada: se nao ha nada pendente, encerra */
            if (pos == 0 && !estourou)
                return 0;
            break;
        }
        if (c == '\n')
            break;

        if (pos < tamanho - 1)
            linha[pos++] = c;
        else
            estourou = 1;
    }

    linha[pos] = '\0';
    return estourou ? -1 : 1;
}

/*
 * Quebra a linha em palavras separadas por espaco ou tab (parsing).
 * A propria linha e modificada: cada separador vira '\0' e args[i]
 * aponta para o inicio de cada palavra. O vetor termina com NULL,
 * que e o formato esperado pelo execvp().
 *
 * Retorna a quantidade de argumentos, ou -1 se houver argumentos demais.
 */
int separar_argumentos(char *linha, char **args, int max_args)
{
    int n = 0;
    char *p = linha;

    while (*p != '\0') {
        while (*p == ' ' || *p == '\t')
            *p++ = '\0';
        if (*p == '\0')
            break;

        if (n >= max_args - 1)
            return -1;
        args[n++] = p;

        while (*p != '\0' && *p != ' ' && *p != '\t')
            p++;
    }

    args[n] = NULL;
    return n;
}
