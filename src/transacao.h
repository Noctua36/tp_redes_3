#ifndef _TRANSACAO_H_ 
#define _TRANSACAO_H_

#include "pacote.h"

typedef struct transacao {
    int timeout_count;
    int timedout;
    int terminada;
    int arquivoAberto;
    int posicaoArquivo;
    char* bufferArquivo;
    int tamBufferArquivo;
    unsigned short numBloco;
    codigoErro codErro;
    char *mensagemErro;
} transacao;

#endif /* _TRANSACAO_H_ */