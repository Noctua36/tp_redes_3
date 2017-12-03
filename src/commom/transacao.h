#ifndef _TRANSACAO_H_ 
#define _TRANSACAO_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "tp_socket.h"
#include "pacote.h"
#include "lista_linkada.h"

#define TIMEOUT 200 // em segundos

typedef struct transacao {
    int socketFd;
    int cargaUtilPacoteDados;
    struct sockaddr_in toAddr;
    char* buf;
    pacote *recebido;
    pacote *envio;
    int timedoutCount;
    int timedout;
    int numBlocoEsperado;
    int tamJanela;
    int qtdNaJanela;
    nodulo* janelaDeslizante;
    int arquivoAberto;
    char* nomeArquivo;
    FILE* arquivo;
    int posicaoArquivo;
    int tamBufferArquivo;
    uint16_t numBloco;
    uint8_t codErro;
    char *mensagemErro;
} transacao;

transacao* inicializaTransacao(int, int, int);
void destroiTransacao();
int validaMensagem(char*);
int extraiCRCDaMensagem(char*);

#endif /* _TRANSACAO_H_ */