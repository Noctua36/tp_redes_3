#include "transacao.h"
#include "crc.h"

transacao* criaTransacaoVazia(int tamMsg, int porta){
  transacao *t = malloc(sizeof *t);

  t->socketFd = 0;
  t->cargaUtilPacoteDados = (int)(tamMsg - sizeof(t->envio->opcode) - sizeof(t->envio->numBloco)); //TODO: adicionar CRC
  t->timeoutCount = 0;
  t->timedout = 0;
  t->numBlocoEsperado = 1;
  t->arquivoAberto = 0;
  t->posicaoArquivo = 0;
  t->tamBufferArquivo = 0;
  t->numBloco = (uint16_t)0;
  t->codErro = (uint8_t)SEM_ERRO;

  t->mensagemErro = calloc(TAM_MSG_ERRO, sizeof(t->mensagemErro));
  t->nomeArquivo = calloc(TAM_NOME_ARQUIVO, sizeof(t->nomeArquivo));
  t->buf = calloc(tamMsg, sizeof t->buf);
  
  if (t == NULL || t->nomeArquivo == NULL || t->mensagemErro == NULL || t->buf == NULL) {
      perror("Falha ao alocar memoria para transacao.");
      exit(EXIT_FAILURE);
  }

  // cria socket em porta aleatória e armazena o respectivo file descriptor
  t->socketFd = tp_socket(porta);
  //TODO: veriricar erro no retorno do socket
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
  int crcRecebido = extraiCRCDaMensagem(msg);
  int crcCalc = calculaCRC(msg);
  return crcCalc == crcRecebido;
}

int extraiCRCDaMensagem(char *msg){
  return 10; //TODO: implementar
}