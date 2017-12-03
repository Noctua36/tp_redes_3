#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/in.h>
#include "fsmCliente.h"
#include "../commom/tp_socket.h"
#include "../commom/arquivo.h"
#include "../commom/pacote.h"
#include "../commom/transacao.h"

#define DEBUG
//#define IMPRIME_DADOS_DO_PACOTE
//#define STEP

#ifdef STEP
void aguardaEnter();
#endif

#define TAM_PORTA 6
#define TAM_NOME_ARQUIVO 32
#define TAM_HOST 16

long getTime();
long timeDiff(long, long);
void inicializa(int*, char**);
void carregaParametros(int*, char**,  char*, short int*, char*, int*, short int*);
void estadoEnviaReq(int*);
void estadoRecebeArq(int*);
void estadoErro(int*);
void estadoEnviaAck(int*);      
void estadoTermino(int*);

int tamMaxMsg;
transacao *t;
short int porta;
short int tamJanela;
char host[TAM_HOST];
int contaBytes = 0;
//variaveis para o tempo

struct timeval inicio_tempo, fim_tempo;
long t_total;
long t_t0;
long t_tf;

int main(int argc, char* argv[]){
  printf("Cliente iniciado\n");
  //inicializa programa: carrega parâmetros, inicializa variáveis, aloca memória...
  inicializa(&argc, argv);
  
  // double comeco, duracao;
  int estadoAtual = ESTADO_ENVIA_REQ;
  int operacao; 
  // opera FSM que rege o comportamento do sistema
  while (1){
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

  exit(EXIT_SUCCESS);
}

void estadoEnviaReq(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] ENVIA_REQ\n");
  #endif
  int status;

  t->envio = criaPacoteVazio();  
  t->envio->opcode = (uint8_t)REQ;
  strcpy(t->envio->nomeArquivo, t->nomeArquivo);
  montaMensagemPeloPacote(t->buf, t->envio);

  // forma endereço para envio do pacote ao servidor
  if (tp_build_addr(&t->toAddr, host, porta) < 0){
      perror("Erro no envio do pacote de requisicao de arquivo");
      // TODO: tratar erro
  }

  #ifdef DEBUG
    printf("[DEBUG] Pacote a ser enviado:\n");
    imprimePacote(t->envio, 0);
  #endif
  #ifdef STEP
    aguardaEnter();
  #endif
  
  // inicia a contagem do tempo
  t_t0 = getTime();
  //envia requisição ao servidor
  status = tp_sendto(t->socketFd, t->buf, tamMaxMsg, &t->toAddr);

  // verifica estado do envio
  if (status > 0){
    *operacao = OPERACAO_OK;
  } 
  else {
    *operacao = OPERACAO_NOK;
  }
  destroiPacote(t->envio);
}

void estadoRecebeArq(int *operacao){
    #ifdef DEBUG
      printf("\n[FSM] RECEBE_ARQ\n");
    #endif
    
    int bytesRecebidos = 0;
    bytesRecebidos = tp_recvfrom(t->socketFd, t->buf, tamMaxMsg, &t->toAddr);
    contaBytes += bytesRecebidos;
    montaPacotePelaMensagem(t->recebido, t->buf, bytesRecebidos);
    
    #ifdef DEBUG
      printf("Pacote recebido:\n");
      imprimePacote(t->recebido, 1);
    #endif

  if (t->recebido->opcode == (uint8_t)DADOS){
    // verifica se bloco é o esperado
    if (t->recebido->numBloco != t->numBlocoEsperado){
      // ignora mensagem
      *operacao = OPERACAO_IGNORA;
      return;
    }
    if (!t->arquivoAberto){
       t->arquivo = abreArquivoParaEscrita(t->nomeArquivo);
       t->arquivoAberto = t->arquivo != NULL;
    }     
    escreveBytesEmArquivo(t->recebido->dados, t->arquivo , t->recebido->cargaUtil);
    *operacao = OPERACAO_OK;
    t->numBlocoEsperado++;
    return;
  }
  if (t->recebido->opcode == (uint8_t)FIM){
    fechaArquivo(t->arquivo);
    *operacao = OPERACAO_TERMINO_ARQ;
    t_tf = getTime();
    t_total = timeDiff(t_t0, t_tf);
    return;
  }
  // carrega erro na transação
  t->codErro = t->recebido->codErro;
  strcpy(t->mensagemErro, t->recebido->mensagemErro);
  *operacao = OPERACAO_NOK;
}

void estadoErro(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] ERRO\n");
  #endif
  //exibe mensagem de erro
  printf("\n[ERRO]> codigo: %d\nmensagem: %s\n", t->codErro, t->mensagemErro);
  destroiTransacao(t);
}

void estadoEnviaAck(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] ENVIA_ACK\n");
  #endif
  
  int status;
  t->envio = criaPacoteVazio(tamMaxMsg);
  t->envio->opcode = (uint8_t)ACK;
  t->envio->numBloco++;
  montaMensagemPeloPacote(t->buf, t->envio);

  #ifdef DEBUG
    printf("[DEBUG] Pacote a ser enviado:\n");
    imprimePacote(t->envio, 0);
  #endif
  #ifdef STEP
    aguardaEnter();
  #endif

  status = tp_sendto(t->socketFd, t->buf, tamMaxMsg, &t->toAddr);

  // verifica estado do envio
  if (status > 0){
    *operacao = OPERACAO_OK;
  } 
  else {
    *operacao = OPERACAO_NOK;
  }
  destroiPacote(t->envio);
}

void estadoTermino(int *operacao){
  #ifdef DEBUG  
    printf("\n[FSM] TERMINO\n");
  #endif
  printf("Saindo...\n");
  //imprime os parametros exigidos
  printf("Buffer = \%d byte(s), \%10.2f kbps (\%u bytes em \%3.6f s)\n", tamMaxMsg, ((float)contaBytes/ (float)t_total), contaBytes, (float)t_total);
  exit(EXIT_SUCCESS);
}

void inicializa(int *argc, char* argv[]){
  char *nomeArquivo = calloc(TAM_NOME_ARQUIVO, sizeof nomeArquivo);
  // alimenta numero da porta e tamanho do buffer pelos parametros recebidos
  carregaParametros(argc, argv, host, &porta, nomeArquivo, &tamMaxMsg, &tamJanela);

  t = inicializaTransacao(tamMaxMsg, 0, tamJanela); // TODO: verificar se nao é porta inves de 0

  strcpy(t->nomeArquivo, nomeArquivo);
  free(nomeArquivo);
  
  // aloca memória para buffer
  t->buf = calloc(tamMaxMsg, sizeof t->buf);
  if (t->buf == NULL){
    perror("Falha ao alocar memoria para buffer.");
    exit(EXIT_FAILURE);
  }

  // chamada de função de inicialização para ambiente de testes
  tp_init();
 
  
}

// UTIL
void carregaParametros(int* argc, char** argv, char* host, short int* porta, char* arquivo, int* tamBuffer, short int* tamJanela){
  #ifdef DEBUG
    printf("[DEBUG] numero de parametros recebidos: %d\n", *argc);
  #endif
  char *ultimoCaractere;
  // verifica se programa foi chamado com argumentos corretos
  if (*argc != 6){
    fprintf(stderr, "ERRO-> parametros invalidos! Uso: %s [host] [porta] [nome_arquivo] [tam_buffer] [tam_janela]\n", argv[0]);
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

  *tamJanela = atoi(argv[5]);
  
  #ifdef DEBUG
    printf("[DEBUG] Parametros recebidos-> host: %s porta: %d nome_arquivo: %s tamBuffer: %d tamJanela: %d\n", host, *porta, arquivo, *tamBuffer, *tamJanela);
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
  } 
  else {
    temp = end - start;
  }
  return temp;
}

#ifdef STEP
void aguardaEnter(){
  printf("pressione Enter para continuar...");
  while (getchar() != '\n');
}
#endif