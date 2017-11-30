#include "transacao.h"
#include "crc.h"

transacao* criaTransacaoVazia(){
    transacao *t = malloc(sizeof *t);

    t->timeoutCount = 0;
    t->timedout = 0;
    t->terminada = 0;
    t->arquivoAberto = 0;
    t->posicaoArquivo = 0;
    t->tamBufferArquivo = 0;
    t->numBloco = (uint16_t)0;
    t->codErro = (uint8_t)SEM_ERRO;

    t->mensagemErro = malloc(TAM_MSG_ERRO * sizeof(char));
    t->nomeArquivo = malloc(TAM_NOME_ARQUIVO * sizeof(char));

    if (t == NULL || t->nomeArquivo == NULL || t->mensagemErro == NULL) {
        perror("Falha ao alocar memoria para transacao.");
        exit(EXIT_FAILURE);
    }

    return t;
}

// verifica se mensagem é valida a partir da verificação do crc recebido
int validaMensagem(char *msg, int crc){
  int crcCalc = calculaCRC(msg);
  return crcCalc == crc;
}