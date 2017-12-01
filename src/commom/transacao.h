#ifndef _TRANSACAO_H_ 
#define _TRANSACAO_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "pacote.h"

#define TIMEOUT 2 // em segundos

typedef struct transacao {
    int socketFd;
    // int tamMsg;
    int cargaUtilPacoteDados;
    struct sockaddr_in toAddr;
    char* buf;
    pacote *recebido;
    pacote *envio;
    int timeoutCount;
    int timedout;
    //int envioCompleto;
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
void destroiTransacao();
int validaMensagem(char*);
int extraiCRCDaMensagem(char*);

#endif /* _TRANSACAO_H_ */