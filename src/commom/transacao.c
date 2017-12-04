#include "transacao.h"
#include "crc.h"

struct timeval timeout; 

transacao* inicializaTransacao(int tamMaxMsg, int porta, int tamJanela){
  timeout.tv_sec = TIMEOUT;
  timeout.tv_usec = 0;
  transacao *t = malloc(sizeof *t);

  t->socketFd = 0;
  t->cargaUtilPacoteDados = (int)(tamMaxMsg - sizeof(t->envio->opcode) - sizeof(t->envio->numBloco)); //TODO: adicionar CRC
  t->timedoutCount = 0;
  t->timedout = 0;
  t->numBlocoEsperado = 1;
  t->tamJanela = tamJanela;
  t->qtdNaJanela = 0;
  t->arquivoAberto = 0;
  t->posicaoArquivo = 0;
  t->tamBufferArquivo = 0;
  t->numBloco = (uint16_t)0;
  t->ultimoNumBloco = (uint16_t)0;
  t->codErro = (uint8_t)SEM_ERRO;

  t->mensagemErro = calloc(TAM_MSG_ERRO, sizeof(t->mensagemErro));
  t->nomeArquivo = calloc(TAM_NOME_ARQUIVO, sizeof(t->nomeArquivo));
  t->bufEnvio = calloc(tamMaxMsg, sizeof t->bufEnvio);
  t->bufRecebimento = calloc(tamMaxMsg, sizeof t->bufRecebimento);
  // t->janelaDeslizante = malloc(sizeof t->janelaDeslizante);

  if (t == NULL || t->nomeArquivo == NULL || t->mensagemErro == NULL || t->bufRecebimento == NULL || t->bufEnvio == NULL){
      perror("Falha ao alocar memoria para transacao.");
      exit(EXIT_FAILURE);
  }

  // cria socket em porta aleatória e armazena o respectivo file descriptor
  t->socketFd = tp_socket(porta);
  
  // verifica sucesso na criação do socket
  if(t->socketFd < 0){
    destroiTransacao(t);
    perror("Falha ao iniciar socket.");
    exit(EXIT_FAILURE); // TODO: reseta?
  }

  // inicia temporizador de timeout
  if (setsockopt (t->socketFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
    perror("falha ao definir timeout para socket.\n");
    exit(EXIT_FAILURE); // TODO: reseta?
  }

  t->recebido = criaPacoteVazio();

  return t;
}

void destroiTransacao(transacao *t){
  close(t->socketFd);
  free(t->mensagemErro);
  free(t->nomeArquivo);
  free(t->bufEnvio);
  free(t->bufRecebimento);
  esvaziaLista(t->janelaDeslizante);
  free(t->janelaDeslizante);
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