#include "transacao.h"
#include "crc.h"
#include <string.h>

transacao* inicializaTransacao(int tamMaxMsg, int porta, int tamJanela){
  transacao *t = malloc(sizeof *t);

  t->socketFd = 0;
  t->cargaUtilPacoteDados = (int)(tamMaxMsg - sizeof(t->envio->opcode) - sizeof(t->envio->numBloco)); //TODO: adicionar CRC
  t->timeoutCount = 0;
  t->timedout = 0;
  t->numBlocoEsperado = 1;
  t->tamJanela = tamJanela;
  t->arquivoAberto = 0;
  t->posicaoArquivo = 0;
  t->tamBufferArquivo = 0;
  t->numBloco = (uint16_t)0;
  t->codErro = (uint8_t)SEM_ERRO;

  t->mensagemErro = calloc(TAM_MSG_ERRO, sizeof(t->mensagemErro));
  t->nomeArquivo = calloc(TAM_NOME_ARQUIVO, sizeof(t->nomeArquivo));
  t->buf = calloc(tamMaxMsg, sizeof t->buf);
  
  if (t == NULL || t->nomeArquivo == NULL || t->mensagemErro == NULL || t->buf == NULL){
      perror("Falha ao alocar memoria para transacao.");
      exit(EXIT_FAILURE);
  }

  // cria socket em porta aleatória e armazena o respectivo file descriptor
  t->socketFd = tp_socket(porta);
  
  // verifica sucesso na criação do socket
  if(t->socketFd < 0){
    destroiTransacao(t);
    perror("Falha ao iniciar socket.");
    exit(EXIT_FAILURE);
  }

  t->recebido = criaPacoteVazio();

  return t;
}

void destroiTransacao(transacao *t){
  close(t->socketFd);
  free(t->mensagemErro);
  free(t->nomeArquivo);
  free(t->buf);
  free(t);
  t = NULL;
}

// verifica se mensagem é valida a partir da verificação do crc recebido
int validaMensagem(char *msg){
  uint32_t crcRecebido = extraiCRCDaMensagem(msg);
  uint32_t crcCalc = calculaCRC(msg, strlen(msg));
  return crcCalc == crcRecebido;
}

uint32_t extraiCRCDaMensagem(char *msg){
  int i;
  char crc[8];
  for (i = 0; i < 7; i++)
    crc[i] = msg[i];
    msg[0] = msg[8];
  return atoi(crc); //TODO: implementar
}