#ifndef _TRANSACAO_H_ 
#define _TRANSACAO_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "pacote.h"

typedef struct transacao {
    int timeoutCount;
    int timedout;
    int terminada;
    int arquivoAberto;
    char* nomeArquivo;
    FILE* arquivo;
    int posicaoArquivo;
    // char* bufferArquivo;
    int tamBufferArquivo;
    uint16_t numBloco;
    uint8_t codErro;
    char *mensagemErro;
} transacao;

transacao* criaTransacaoVazia();
int validaMensagem(char*, int);

#endif /* _TRANSACAO_H_ */