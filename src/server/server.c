
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
#include "../commom/tp_socket.h"
#include "fsmServidor.h"
#include "../commom/arquivo.h"
#include "../commom/pacote.h"
#include "../commom/transacao.h"
// #include <sys/socket.h>
// #include <netdb.h>
// #include <netinet/in.h>

#define DEBUG

#define TIMEOUT 2 // em segundos

// protótipo das funções
void carregaParametros(int*, char**, short int*, int*);
void limpaBuffer(char*);
void estadoStandBy(int*);
void estadoAguardaAck(int*);
void estadoEnvia(int*);
void estadoReseta(int*);
void estadoErro(int*);

int sock, tam_msg;
short int porta;
struct sockaddr_in cli_addr;
char* buf;
pacote *recebido;
transacao *t;
struct timeval timeout;      

int main(int argc, char* argv[]){
  
  // alimenta número da porta e tamanho do buffer pelos parâmetros recebidos
  carregaParametros(&argc, argv, &porta, &tam_msg);
  
  // aloca memória para buffer
  buf = malloc(sizeof *buf * tam_msg);
  if (buf == NULL){
    perror("Falha ao alocar memoria para buffer.");
    exit(EXIT_FAILURE);
  }
 
  timeout.tv_sec = TIMEOUT;
  timeout.tv_usec = 0;

  // chamada de função de inicialização para ambiente de testes
  tp_init();
 
  // cria socket e armazena o respectivo file descriptor
  sock = tp_socket(porta);

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
  bytesRecebidos = tp_recvfrom(sock, buf, tam_msg, &cli_addr);                     
        
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
  if ((opCode)recebido->opcode == REQ)
    *operacao = OPERACAO_REQ_RECEBIDA;
  else 
    *operacao = OPERACAO_NOK;
}

void estadoEnvia(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] ENVIA\n");
  #endif

  int status;
  pacote *envio;
  // verifica se arquivo já está aberto
  if (!t->arquivoAberto){
    t->arquivo = abreArquivoParaLeitura(recebido->nomeArquivo);
    // TODO: verificar se arquivo existe e tratar erro
    t->arquivoAberto = 1;
    #ifdef DEBUG
      printf("Abrindo arquivo...%s", recebido->nomeArquivo);
    #endif
  } 

  envio = criaPacoteVazio();
  
  //verifica se chegou ao fim do arquivo
  if(feof(t->arquivo))
  {
    #ifdef DEBUG
    printf("Fim do arquivo\n");
    #endif
    //envia mensagem sinalizando o final do arquivo
    envio->opcode = (uint8_t)FIM;
    montaBufferPeloPacote(buf, envio);
    status = tp_sendto(sock, buf, strlen(buf), &cli_addr);
    
      // verifica estado do envio
      if (status > 0) {
        *operacao = OPERACAO_OK;
      } else {
        *operacao = OPERACAO_NOK;
      }
      destroiPacote(envio);
      
    
    exit(EXIT_SUCCESS);
  }
  // cria pacote para envio
  envio->opcode = (uint8_t)DADOS;
  envio->numBloco = t->numBloco++;
  int cargaUtil = tam_msg - sizeof(envio->opcode) - sizeof(envio->numBloco);
  leBytesDeArquivo(envio->dados, t->arquivo, cargaUtil);
  montaBufferPeloPacote(buf, envio);

  #ifdef DEBUG
    printf("Buffer a ser enviado: \n");
    imprimeBuffer(buf);
    printf("Pacote a ser enviado: \n");
    imprimePacote(envio);
  #endif
  status = tp_sendto(sock, buf, tam_msg, &cli_addr);
  #ifdef DEBUG
    printf("Bytes enviados: %d \n", status);
  #endif
  // verifica estado do envio
  if (status > 0) {
    *operacao = OPERACAO_OK;
  } else {
    *operacao = OPERACAO_NOK;
  }
  destroiPacote(envio);
  
}

void estadoAguardaAck(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] AGUARDA_ACK\n");
  #endif

  int bytesRecebidos = 0;
  
  limpaBuffer(buf);
  // inicia temporizador de timeout
  if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
    perror("falha ao definir timeout para socket.\n");
    // TODO: tratar erro
  }
  bytesRecebidos = tp_recvfrom(sock, buf, tam_msg, &cli_addr);                     
  if (bytesRecebidos == -1){
    perror("ERRO-> Nao foi possivel ler comando do socket: %s]\n");
    *operacao = OPERACAO_NOK;
  }

  montaPacotePeloBuffer(recebido, buf);
  #ifdef DEBUG
  
  printf("Buffer Recebido\n");
  imprimeBuffer(buf);
  printf("Pacote Recebido\n");
  imprimePacote(recebido);

  #endif

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
  memset(b, 0, tam_msg);
}