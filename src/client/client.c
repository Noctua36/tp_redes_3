#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include "tp_socket.h"
#include "fsmCliente.h"
#include "../commom/arquivo.h"
#include "../commom/pacote.h"
#include "../commom/transacao.h"
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netdb.h>

// #include <unistd.h>
// #include <arpa/inet.h> //INET_NTOP
// #include <stdbool.h>

#define DEBUG

// #ifdef DEBUG
// #define DEBUG_TEST 1
// #else
// #define DEBUG_TEST 0
// #endif
// #define debug_print(fmt, ...) 
//   do { if (DEBUG_TEST) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

#define TAM_PORTA 6
#define TAM_NOME_ARQUIVO 32
#define TAM_HOST 16

long getTime();
long timeDiff(long, long);
void carregaParametros(int*, char**,  char*, short int*, char*, int*);
void estadoEnviaReq(int*);
void estadoRecebeArq(int*);
void estadoErro(int*);
void estadoEnviaAck(int*);      
void estadoTermino(int*);

int sock, mtu;
pacote *envio;
transacao *t;
char* buf; // TODO: avaliar se buffer pode ser transferido para dentro da struct transacao
short int porta;
char host[TAM_HOST];
so_addr *saddr;

int main(int argc, char* argv[]){
  // long int bytesRecebidos = 0;
  char nomeArquivo[TAM_NOME_ARQUIVO];
  
  //TODO: avaliar se mtu deve vir da linha de comando
  //mtu = tp_mtu();
  // double comeco, duracao;

  // alimenta numero da porta e tamanho do buffer pelos parametros recebidos
  carregaParametros(&argc, argv, host, &porta, nomeArquivo, &mtu);

  // aloca memória para buffer
  buf = malloc(sizeof *buf * mtu);
  if (buf == NULL){
    perror("Falha ao alocar memoria para buffer.");
    exit(EXIT_FAILURE);
  }

  saddr = malloc(sizeof(so_addr));

  // chamada de função de inicialização para ambiente de testes
  tp_init();

  int estadoAtual = ESTADO_ENVIA_REQ;
  int operacao; 

  envio = criaPacoteVazio(mtu);
  t = criaTransacaoVazia();
  strcpy(t->nomeArquivo, nomeArquivo);
  while(1){
    switch(estadoAtual){
      case ESTADO_ENVIA_REQ:
        estadoEnviaReq(&operacao);
        break;
      case ESTADO_RECEBE_ARQ:
        estadoRecebeArq(&operacao);
        break;
      case ESTADO_ERRO:
        estadoErro(&operacao);
        break;
      case ESTADO_ENVIA_ACK:
        estadoEnviaAck(&operacao);
        break;
      case ESTADO_TERMINO:
        estadoTermino(&operacao);
      break;
    }
    transita(&estadoAtual, &operacao);
  }

  free(saddr);
  exit(EXIT_SUCCESS);
}

void estadoEnviaReq(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] ENVIA_REQ\n");
  #endif
  int status;
  // TODO: verificar possibilidade de criar pacote 'envio', enviar e excluí-lo aqui dentro desta função
  envio->opcode = (uint8_t)REQ;
  strcpy(envio->nomeArquivo, t->nomeArquivo);
  montaBufferPeloPacote(buf, envio);

  // cria socket
  sock = tp_socket(0);
  
  #ifdef DEBUG
    printf("\n[DEBUG] socket: %d, mtu: %d\n", sock, mtu);
    imprimeBuffer(buf);
  #endif

  // forma endereço para envio do pacote ao servidor
  if (tp_build_addr(saddr, host, porta) < 0){
      perror("Erro no envio do pacote de requisicao de arquivo");
      // TODO: tratar erro
  }

  status = tp_sendto(sock, buf, mtu, saddr);

  // inserir verificação se foi final do arquivo

  #ifdef DEBUG
    printf("\n[DEBUG] status: %d\n", status);
  #endif
  // verifica estado do envio
  if (status > 0) {
    *operacao = OPERACAO_OK;
  } else {
    *operacao = OPERACAO_NOK;
  }
  destroiPacote(envio);
}

void estadoRecebeArq(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] RECEBE_ARQ\n");
  #endif
  FILE *fp = NULL;

  int bytesRecebidos = 0;
  bytesRecebidos = tp_recvfrom(sock, buf, mtu, saddr);
  pacote *recebido = criaPacoteVazio();
  montaPacotePeloBuffer(recebido, buf);

  
  #ifdef DEBUG
    printf("BytesRecebidos: %d\n", bytesRecebidos);
    printf("Buffer recebido:\n");
    imprimeBuffer(buf);
    printf("Pacote recebido:\n");
    imprimePacote(recebido);
  #endif

  if(recebido->opcode == (uint8_t)DADOS)
  {
    if (fp == NULL)
    {
      FILE *fp = abreArquivoParaEscrita(recebido->nomeArquivo);
      fprintf(fp, "%s" , recebido->dados);
    }
    else
    {
      fprintf(fp, "%s" , recebido->dados);
    }
  }
}

void estadoErro(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] ERRO\n");
  #endif
}

void estadoEnviaAck(int *operacao){
  int status;
  
  #ifdef DEBUG
    printf("\n[FSM] ENVIA_ACK\n");
  #endif
  //envio = criaPacoteVazio();
  envio->opcode = (uint8_t)ACK;
  envio->numBloco = t->numBloco ++;
  montaBufferPeloPacote(buf, envio);
  
  status = tp_sendto(sock, buf, mtu, saddr);

  #ifdef DEBUG
  printf("\n[DEBUG] status: %d\n", status);
  printf("\n[DEBUG] socket: %d, mtu: %d\n", sock, mtu);
  //imprimeBuffer(buf);
  #endif
  // verifica estado do envio
  if (status > 0) {
  *operacao = OPERACAO_OK;
  } else {
  *operacao = OPERACAO_NOK;
  }
  destroiPacote(envio);
}

void estadoTermino(int *operacao){
  #ifdef DEBUG  
    printf("\n[FSM] TERMINO\n");
  #endif
  printf("Saindo...\n");
  exit(EXIT_SUCCESS);
}

// UTIL
void carregaParametros(int* argc, char** argv, char* host, short int* porta, char* arquivo, int* tamBuffer){
  #ifdef DEBUG
    printf("[DEBUG] numero de parametros recebidos: %d\n", *argc);
  #endif
  char *ultimoCaractere;
  // verifica se programa foi chamado com argumentos corretos
  if (*argc != 5){
    fprintf(stderr, "ERRO-> parametros invalidos! Uso: %s [host] [porta] [nome_arquivo] [tam_buffer]\n", argv[0]);
    exit(EXIT_FAILURE);
  } 
  else {
    errno = 0;
    *tamBuffer = strtoul(argv[4], &ultimoCaractere, 10);
    if ((errno == ERANGE && (*tamBuffer == LONG_MAX || *tamBuffer == LONG_MIN)) || (errno != 0 && *tamBuffer == 0)){
      perror("strtoul");
      exit(EXIT_FAILURE);
    }
  }
  *porta = atoi(argv[2]);

  memset(host, '\0', TAM_HOST);
  strcpy(host, argv[1]);

  memset(arquivo, '\0', TAM_NOME_ARQUIVO);
  strcpy(arquivo, argv[3]);
  
  #ifdef DEBUG
    printf("[DEBUG] Parametros recebidos-> host: %s porta: %d nome_arquivo: %s tamBuffer: %d\n", host, *porta, arquivo, *tamBuffer);
  #endif
}

long getTime(){
    struct timespec tempo;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tempo);
    return tempo.tv_nsec;
}
  
// retorna diferença em nanosegundos
long timeDiff(long start, long end){
  long temp;
  if ((end - start) < 0){
    temp = 1000000000 + end - start;
  } else {
    temp = end - start;
  }
  return temp;
}