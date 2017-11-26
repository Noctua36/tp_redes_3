#ifndef _TRANSACAO_H_ 
#define _TRANSACAO_H_

#include <stdio.h>
#include <stdlib.h>
#include "pacote.h"

typedef struct transacao {
    int timeoutCount;
    int timedout;
    int terminada;
    int arquivoAberto;
    FILE* arquivo;
    int posicaoArquivo;
    // char* bufferArquivo;
    int tamBufferArquivo;
    unsigned short numBloco;
    codigoErro codErro;
    char *mensagemErro;
} transacao;

transacao* criaTransacaoVazia();

#endif /* _TRANSACAO_H_ */