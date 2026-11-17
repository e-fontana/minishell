#include <string.h>
#include <unistd.h>

#include "minishell.h"

/*
 * Escreve um texto no descritor indicado (1 = saida padrao, 2 = saida de erro)
 * usando a chamada ao sistema write().
 * O write() pode escrever menos bytes do que foi pedido, por isso o laco.
 */
void escrever(int fd, const char *texto)
{
    size_t falta = strlen(texto);

    while (falta > 0) {
        ssize_t escritos = write(fd, texto, falta);
        if (escritos <= 0)
            return;
        texto += escritos;
        falta -= (size_t)escritos;
    }
}
