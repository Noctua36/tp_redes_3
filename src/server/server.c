
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h> //INET_NTOP
#include <signal.h>
#include "tp_socket.h"
#include "fsmServidor.h"
#include "../commom/arquivo.h"
#include "../commom/pacote.h"
#include "../commom/transacao.h"
// #include <sys/socket.h>
// #include <netdb.h>
// #include <netinet/in.h>

#define DEBUG

#define TIMEOUT 5

// protótipo das funções
void carregaParametros(int*, char**, short int*, int*);
void limpaBuffer(char*);
void timeoutHandler(int);
void estadoStandBy(int*);
void estadoAguardaAck(int*);
void estadoEnvia(int*);
void estadoReseta(int*);
void estadoErro(int*);
// int pacotePedidoArquivo();
// int pacoteRecebeAck();
// int pacoteRecebeInvalido();

int sockServFd, sockCliFd, mtu;
short int porta;
struct sockaddr_in cli_addr;
char* buf;
pacote *recebido;
transacao *t;
struct timeval timeout;      

int main(int argc, char* argv[]){
  
  mtu = tp_mtu();

  // alimenta número da porta e tamanho do buffer pelos parâmetros recebidos
  carregaParametros(&argc, argv, &porta, &mtu);
  
  // aloca memória para buffer
  buf = malloc(sizeof *buf * mtu);
  if (buf == NULL){
    perror("Falha ao alocar memoria para buffer.");
    exit(EXIT_FAILURE);
  }
 
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  // chamada de função de inicialização para ambiente de testes
  tp_init();
 
  // cria socket e armazena o respectivo file descriptor
  sockServFd = tp_socket(porta);

  int estadoAtual = ESTADO_STANDBY;
  int operacao;   

  recebido = criaPacoteVazio();
  t = criaTransacaoVazia();
  while(1){
    switch(estadoAtual){
      case ESTADO_STANDBY:
        estadoStandBy(&operacao);
        break;
      case ESTADO_ENVIA:
        estadoEnvia(&operacao);
        break;
      case ESTADO_AGUARDA_ACK:
        estadoAguardaAck(&operacao);
        break;
      case ESTADO_ERRO:
        estadoErro(&operacao);
        break;
      case ESTADO_RESETA:
        estadoReseta(&operacao);
      break;
    }
    transita(&estadoAtual, &operacao);
  }
  
  exit(EXIT_SUCCESS);
}

void estadoStandBy(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] STAND_BY\n");
  #endif
  int bytesRecebidos = 0;
 
  limpaBuffer(buf);
  bytesRecebidos = tp_recvfrom(sockServFd, buf, mtu, &cli_addr);                     
        
  #ifdef DEBUG
    printf("bytesRecebidos: %d\n", bytesRecebidos);
  #endif

  if (bytesRecebidos == -1){
    perror("ERRO-> Nao foi possivel ler comando do socket: %s]\n");
    *operacao = OPERACAO_NOK;
  }

  montaPacotePeloBuffer(recebido, buf);
  #ifdef DEBUG
    printf("pacote recebido:\n");
    imprimePacote(recebido);
  #endif
  if (recebido->opcode == REQ)
    *operacao = OPERACAO_REQ_RECEBIDA;
  else 
    *operacao = OPERACAO_NOK;
}

void estadoEnvia(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] ENVIA\n");
  #endif

  int socket, status;
  pacote *envio;
  // verifica se arquivo já está aberto
  if (!t->arquivoAberto){
    t->arquivo = abreArquivoParaLeitura(recebido->nomeArquivo);
    // TODO: verificar se arquivo existe e tratar erro
    t->arquivoAberto = 1;
  } 
  else {
    // cria pacote para envio
    envio = criaPacoteVazio();
    envio->opcode = DADOS;
    envio->numBloco = t->numBloco++;
    leBytesDeArquivo(envio->dados, t->arquivo, mtu); // TODO: descontar do mtu bytes utilizados pelo cabeçalho
    montaBufferPeloPacote(buf, envio);

    // cria socket
    socket = tp_socket(porta);
    status = tp_sendto(socket, buf, mtu, &cli_addr);

    // verifica estado do envio
    if (status > 0) {
      *operacao = OPERACAO_OK;
    } else {
      *operacao = OPERACAO_NOK;
    }
    destroiPacote(envio);
  }


}

void estadoAguardaAck(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] AGUARDA_ACK\n");
  #endif

  int bytesRecebidos = 0;
  
  limpaBuffer(buf);
  // inicia temporizador de timeout
  // signal(SIGALRM, timeoutHandler);
  // alarm(TIMEOUT);
  if (setsockopt (sockServFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
    perror("falha ao definir timeout para socket.\n");
    // TODO: tratar erro
  }
  bytesRecebidos = tp_recvfrom(sockServFd, buf, mtu, &cli_addr);                     
  // cancela timeout
  // alarm(0);        
  if (bytesRecebidos == -1){
    perror("ERRO-> Nao foi possivel ler comando do socket: %s]\n");
    *operacao = OPERACAO_NOK;
  }

  montaPacotePeloBuffer(recebido, buf);
  if (recebido->opcode == ACK){
    // TODO: extrair e avaliar sobre qual numBloco tal ACK é referente
    *operacao = OPERACAO_OK;
  }
  else 
    *operacao = OPERACAO_NOK;
}

void estadoReseta(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] RESETA\n");
  #endif
}

void estadoErro(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] ERRO\n");
  #endif
}

void timeoutHandler(int signo){
  t->timeoutCount++;
  t->timedout = 1;
}

// int pacotePedidoArquivo(){
//   int existe = verificaSeArquivoExiste(pacote.nomeArquivo);
//   if (!existe){
//     pacote.erro = 2; //TODO: tratar erro
//     return OPERACAO_NOK;
//   }
//   abreArquivoParaLeitura(pacote.nomeArquivo);
//   return (existe) ? OPERACAO_OK : OPERACAO_NOK;
// }

// int pacoteRecebeAck(){return 0;}
// int pacoteRecebeInvalido(){return 0;}

// UTIL
void carregaParametros(int* argc, char** argv, short int* porta, int* tamBuffer){
  char *ultimoCaractere;
  // verifica se programa foi chamado com argumentos corretos
  if (*argc != 3){
    fprintf(stderr, "ERRO-> parametros invalidos! Uso: %s [porta] [tam_buffer]\n", argv[0]);
    exit(EXIT_FAILURE);
  } 
  else {
    errno = 0;
    *tamBuffer = strtoul(argv[2], &ultimoCaractere, 10);
    if ((errno == ERANGE && (*tamBuffer == LONG_MAX || *tamBuffer == LONG_MIN)) || (errno != 0 && *tamBuffer == 0)){
      perror("Erro ao converter tamanho do buffer para numero");
      exit(EXIT_FAILURE);
    }
    *porta = atoi(argv[1]);
  }
  #ifdef DEBUG
    printf("[DEBUG] Parametros recebidos-> porta: %d tamBuffer: %d\n", *porta, *tamBuffer);
  #endif
}

void limpaBuffer(char *b){
  memset(b, 0, mtu);
}