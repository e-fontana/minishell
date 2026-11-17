#ifndef MINISHELL_H
#define MINISHELL_H

#define TAM_LINHA 1024 /* tamanho maximo de uma linha de comando */
#define MAX_ARGS 64    /* numero maximo de argumentos (contando o comando) */

/* saida.c */
void escrever(int fd, const char *texto);

/* leitura.c */
int ler_linha(char *linha, int tamanho);
int separar_argumentos(char *linha, char **args, int max_args);

/* execucao.c */
void executar_comando(char **args);

#endif
