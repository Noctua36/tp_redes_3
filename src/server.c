#include "tp_socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include "fsmServidor.h"
#include <sys/types.h>
#include <sys/stat.h>
// #include <sys/socket.h>
// #include <netdb.h>
// #include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h> //INET_NTOP
// #include <stdbool.h>

#define DEBUG

// protótipo das funções
void carregaParametros(int*, char**, short int*, long int*);
int verificaSeArquivoExiste(char*);
void estadoStandBy(int*);
void limpaPacote();
void preenchePacote();
void estadoRecebe(int*);
void estadoAguarda(int*);
void estadoEnvia(int*);
void estadoReseta(int*);
int pacotePedidoArquivo();
int pacoteRecebeAck();
int pacoteRecebeInvalido();
void abreArquivoParaLeitura(char*);


int sockServFd, sockCliFd;
struct sockaddr_in local_addr;
char* buffer;

int main(int argc, char* argv[]){
  
  short int porta;

  // alimenta número da porta e tamanho do buffer pelos parâmetros recebidos
  carregaParametros(&argc, argv, &porta, &tamBuffer);
  
  // aloca memória para buffers
  buffer = malloc(sizeof *buffer * tamBuffer);
  if (buffer == NULL){
    perror("Falha ao alocar memoria para buffer.");
    exit(EXIT_FAILURE);
  }
  pacote.buffer = malloc(sizeof pacote.buffer * tamBuffer);
  if (pacote.buffer == NULL){
    perror("Falha ao alocar memoria para buffer.");
    exit(EXIT_FAILURE);
  }
  pacote.nomeArquivo = malloc(sizeof pacote.nomeArquivo * 20);
  if (pacote.nomeArquivo == NULL){
    perror("Falha ao alocar memoria para buffer.");
    exit(EXIT_FAILURE);
  }
  // chamada de função de inicialização para ambiente de testes
  tp_init();
 
  // cria socket e armazena o respectivo file descriptor
  sockServFd = tp_socket(porta);

  int estadoAtual = ESTADO_STANDBY;
  int operacao;   

  while(estadoAtual != ESTADO_TERMINO){
    switch(estadoAtual){
      case ESTADO_STANDBY:
        estadoStandBy(&operacao);
        break;
      case ESTADO_RECEBE:
        estadoRecebe(&operacao);
        break;
      case ESTADO_AGUARDA:
        estadoAguarda(&operacao);
        break;
      case ESTADO_ENVIA:
        estadoEnvia(&operacao);
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
  int bytesRecebidos = 0;
  #ifdef DEBUG
    printf("\n\nServidor iniciado\n");
  #endif

  bytesRecebidos = tp_recvfrom(sockServFd, pacote.buffer, tamBuffer, &local_addr);                     
        
  if (bytesRecebidos == -1){
    perror("ERRO-> Nao foi possivel ler comando do socket: %s]\n");
    *operacao = OPERACAO_NOK;
  }
  *operacao = OPERACAO_OK;
}

void estadoRecebe(int *operacao){
  preenchePacote();
  switch(pacote.tipo){
    case PEDIDO_ARQUIVO:
      *operacao = pacotePedidoArquivo();
      break;
    case ACK:
      *operacao = pacoteRecebeAck();
      break;
    // case ERRO:
    //   *operacao = pacoteRecebeErro();
    //   break;
    default:
      *operacao = pacoteRecebeInvalido();
  }
}

void estadoAguarda(int *operacao){

}

void estadoEnvia(int *operacao){

}

void estadoReseta(int *operacao){

}

FILE *arquivo;
int pacotePedidoArquivo(){
  int existe = verificaSeArquivoExiste(pacote.nomeArquivo);
  if (!existe){
    pacote.erro = 2; //TODO: tratar erro
    return OPERACAO_NOK;
  }
  abreArquivoParaLeitura(pacote.nomeArquivo);
  return (existe) ? OPERACAO_OK : OPERACAO_NOK;
}

int pacoteRecebeAck(){return 0;}
int pacoteRecebeInvalido(){return 0;}

void abreArquivoParaLeitura(char *nomeArquivo){
    arquivo = fopen(nomeArquivo, "r");    
    if(arquivo == NULL){
      perror("Não foi possível abrir arquivo para leitura");
    }
}

int leArquivoDePosicao(int posicao){
  // TODO: implementar
  return 0;
}

// UTIL
void carregaParametros(int* argc, char** argv, short int* porta, long int* tamBuffer){
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
    printf("[DEBUG] Parametros recebidos-> porta: %d tamBuffer: %ld\n", *porta, *tamBuffer);
  #endif
}

int verificaSeArquivoExiste(char* nomeArquivo){
  return access(nomeArquivo, F_OK) != -1;
}