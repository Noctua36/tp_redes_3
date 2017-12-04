#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "../commom/tp_socket.h"
#include "../commom/arquivo.h"
#include "../commom/pacote.h"
#include "../commom/transacao.h"
#include "fsmServidor.h"

#define DEBUG
// #define IMPRIME_DADOS_DO_PACOTE
// #define STEP

#define MAX_TIMEOUTS 10

#ifdef STEP
void aguardaEnter();
#endif

// protótipo das funções
void inicializa(int*, char**);
void carregaParametros(int*, char**, short int*, int*, short int*);
int recebePacoteEsperado(uint8_t);
void* recebeAcks();
void deslizaJanela(uint16_t);
void adicionaNaJanela(pacote*);
void limpaBuffer(char*, int);
void estadoStandBy(int*);
void estadoAguardaAck(int*);
void estadoEnvia(int*);
void estadoReseta(int*);
void estadoErro(int*);
void estadoTermino(int*);

short int porta;
short int tamJanela = 3; //  TODO: remover
int tamMaxMsg;
transacao *t;
pthread_t threadRecebeAcks;
pthread_mutex_t mutexJanelaDeslizante = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char* argv[]){
  printf("Inicio\n");
  //inicializa programa: carrega parâmetros, inicializa variáveis, aloca memória...
  inicializa(&argc, argv);
  
  int estadoAtual = ESTADO_STANDBY;
  int operacao;
  // opera FSM que rege o comportamento do sistema
  while (1){
    switch(estadoAtual){
      case ESTADO_STANDBY:
        estadoStandBy(&operacao);
        break;
      case ESTADO_ENVIA:
        estadoEnvia(&operacao);
        break;
      // case ESTADO_AGUARDA_ACK:
      //   estadoAguardaAck(&operacao);
      //   break;
      case ESTADO_ERRO:
        estadoErro(&operacao);
        break;
      case ESTADO_RESETA:
        estadoReseta(&operacao);
        break;
      case ESTADO_TERMINO:
        estadoTermino(&operacao);
      break;
    }
    transita(&estadoAtual, &operacao);
  }
  
  exit(EXIT_SUCCESS);
}

void estadoStandBy(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] STAND_BY\n");
    printf("Aguardando solicitacao...\n");
  #endif
 
  if (recebePacoteEsperado((u_int8_t)REQ)){
    strcpy(t->nomeArquivo, t->recebido->nomeArquivo);
    *operacao = OPERACAO_REQ_RECEBIDA;
  } 
  else {
    #ifdef DEBUG
      printf("Aguardando mensagem de requisicao de arquivo, porem recebido opcode %d.", t->recebido->opcode);
    #endif
    // t->codErro = COD_ERRO_OP_ILEGAL;
    // strcpy(t->mensagemErro, MSG_ERRO_OP_ILEGAL);
    *operacao = OPERACAO_IGNORA;
  }
}

void estadoEnvia(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] ENVIA\n");
    printf("qtd na janela: %d\n",t->qtdNaJanela);
  #endif
  int janelaLivre = 0;
  pthread_mutex_lock(&mutexJanelaDeslizante);
  janelaLivre = t->tamJanela - t->qtdNaJanela > 0;
  pthread_mutex_unlock(&mutexJanelaDeslizante);
  // verifica se janela ainda não está cheia
  if (janelaLivre){
    // certifica que thread para recebimento de acks já foi iniciada
    if (!threadRecebeAcks){
      int erroThread = pthread_create(&threadRecebeAcks, NULL, recebeAcks, NULL);
      if(erroThread){
          perror("Falha ao criar thread para recebimento de acks.");
          exit(EXIT_FAILURE); //TODO: reseta?
      }
    }

    // verifica se arquivo já está aberto
    if (!t->arquivoAberto){
      t->arquivo = abreArquivoParaLeitura(t->nomeArquivo);
      if (t->arquivo == NULL){
        t->codErro = (uint8_t)COD_ERRO_ARQUIVO_NAO_EXISTE; // TODO: diferenciar erros de permissão de leitura e de existância de arquivo
        *operacao = OPERACAO_NOK;
        return;
      }
      else
        t->arquivoAberto = 1;
    } 

    // se arquivo chegou ao fim, não cria mais pacotes de envio, porém aguarda até que todos os ACKs tentam sido recebidos
    if (feof(t->arquivo)){
      if (t->janelaDeslizante == NULL){ // significa que todos os envios foram ACKed
        *operacao = OPERACAO_TERMINO;
      }
      else {
        *operacao = OPERACAO_OK;// mantém no estado
      }
    }
    else {
      t->envio = criaPacoteVazio();
        
      // cria pacote para envio
      t->envio->opcode = (uint8_t)DADOS;
      t->envio->numBloco = ++t->ultimoNumBloco;
      int bytesArquivo = leBytesDeArquivo(t->envio->dados, t->arquivo, t->cargaUtilPacoteDados);
      t->envio->cargaUtil = bytesArquivo;
      montaMensagemPeloPacote(t->bufEnvio, t->envio);

      #ifdef DEBUG
        printf("[DEBUG] Pacote a ser enviado:\n");
        imprimePacote(t->envio, 1);
      #endif
      #ifdef STEP
        aguardaEnter();
      #endif
      // envia parte do arquivo
      int tamMsg = t->envio->cargaUtil + sizeof t->envio->opcode + sizeof t->envio->numBloco;
      adicionaNaJanela(t->envio);
      int status = tp_sendto(t->socketFd, t->bufEnvio, tamMsg, &t->toAddr);
      if (status > 0){
        *operacao = OPERACAO_OK;
      } 
      else {
        // destroiPacote(t->envio);
        *operacao = OPERACAO_NOK;
      }
    }
  }
  if (t->timedoutCount >= MAX_TIMEOUTS){
    printf("num timeouts %d", t->timedoutCount);
    printf("Numero maximo de timeouts alcancado. Envio cancelado.\n");
    *operacao = OPERACAO_ABANDONA;
    return;
  }
  // verifica timeout
  if (t->timedout){
    printf("travei\n");
    // reenvia todos da janela
    nodulo *atual = t->janelaDeslizante;
    while (atual != NULL){
      montaMensagemPeloPacote(t->bufEnvio, atual->pack);
      // envia parte do arquivo
      int tamMsg = atual->pack->cargaUtil + sizeof atual->pack->opcode + sizeof atual->pack->numBloco;
      #ifdef DEBUG
        printf("[DEBUG] Pacote a ser enviado:\n");
        imprimePacote(t->envio, 1);
      #endif
      #ifdef STEP
        aguardaEnter();
      #endif
      /*int status = */tp_sendto(t->socketFd, t->bufEnvio, tamMsg, &t->toAddr);
      atual = atual->proximo;
    }
  }
}

void estadoReseta(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] RESETA\n");
  #endif

  // finaliza thread de recebimento de acks
  pthread_cancel(threadRecebeAcks);

  destroiTransacao(t);
  t = inicializaTransacao(tamMaxMsg, porta, tamJanela);
  *operacao = OPERACAO_OK;
}

void estadoErro(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] ERRO\n");
  #endif
  t->envio = criaPacoteVazio();
  t->envio->opcode = (uint8_t)ERRO;
  t->envio->codErro = (uint8_t)t->codErro;
  strcpy(t->envio->mensagemErro, mensagemDeErroPeloCodigo((codigoErro)t->codErro));

  montaMensagemPeloPacote(t->bufEnvio, t->envio);

  #ifdef DEBUG
    printf("[DEBUG] Pacote a ser enviado:\n");
    imprimePacote(t->envio, 1);
  #endif
  #ifdef STEP
    aguardaEnter();
  #endif
  // envia parte do arquivo
  int tamMsg = sizeof t->envio->opcode + sizeof t->envio->codErro + TAM_MSG_ERRO;
  int status = tp_sendto(t->socketFd, t->bufEnvio, tamMsg, &t->toAddr);
  if (status > 0){
      *operacao = OPERACAO_OK;
    } 
    else {
      *operacao = OPERACAO_NOK;
    }
}

void estadoTermino(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] TERMINO\n");
  #endif
  t->envio = criaPacoteVazio();
  t->envio->opcode = (uint8_t)FIM;
  montaMensagemPeloPacote(t->bufEnvio, t->envio);
  #ifdef DEBUG
    printf("[DEBUG] Pacote a ser enviado:\n");
    imprimePacote(t->envio, 0);
  #endif
  #ifdef STEP
    aguardaEnter();
  #endif
  int status = tp_sendto(t->socketFd, t->bufEnvio, tamMaxMsg, &t->toAddr);
  if (status > 0){
    *operacao = OPERACAO_OK;
  }
  //TODO: tratar erro
  destroiPacote(t->envio);
  exit(EXIT_SUCCESS); // TODO: resetar
}

int recebePacoteEsperado(uint8_t opCodeEsperado){
  limpaBuffer(t->bufRecebimento, tamMaxMsg);
  int bytesRecebidos = tp_recvfrom(t->socketFd, t->bufRecebimento, tamMaxMsg, &t->toAddr);                     
  if (bytesRecebidos == -1){
    // timeout ou outro erro
    //perror("ERRO-> Falha no recebimento de mensagem.\n");
    //TODO: avaliar se cria msg de erro na transacao
    return 0;
  }

  montaPacotePelaMensagem(t->recebido, t->bufRecebimento, bytesRecebidos);
  #ifdef DEBUG
    printf("Recebido mensagem:\n");
    imprimePacote(t->recebido, 0);
  #endif

  if (opCodeEsperado == t->recebido->opcode){
    if (t->recebido->opcode == DADOS){
      //TODO: verificar se verifica CRC apenas para mensagens do tipo DADOS
      int msgIntegra = validaMensagem(t->bufRecebimento);
      if (!msgIntegra){
        #ifdef DEBUG
          printf("Mensagem corrompida recebida!\n");
        #endif
        // mensagem corrompida, apenas ignora, mantendo a FSM no mesmo estado
        return 0;
      }  
    }
    else
      return 1;
  }
  return 0;
}

void* recebeAcks(){
  int resultadoOk;
  uint16_t numBloco;
  while(1){
    resultadoOk = recebePacoteEsperado((uint8_t)ACK);
    if (resultadoOk){
      numBloco = t->recebido->numBloco;
      deslizaJanela(numBloco);
      t->timedout = 0;
      t->timedoutCount = 0;
    }
    else {
      t->timedout = 1;
      t->timedoutCount++;
    }
  }
}

void deslizaJanela(uint16_t numBloco){
  pthread_mutex_lock(&mutexJanelaDeslizante);
  while (t->janelaDeslizante != NULL && t->janelaDeslizante->pack != NULL && (numBloco > t->janelaDeslizante->pack->numBloco)){
    // desliza janela até próximo envio não reconhecido
    pacote* p = pop(&t->janelaDeslizante);
    destroiPacote(p);
    t->qtdNaJanela--;
  }
  pthread_mutex_unlock(&mutexJanelaDeslizante);
}

void adicionaNaJanela(pacote *p){  
  pthread_mutex_lock(&mutexJanelaDeslizante);
    push(&t->janelaDeslizante, p);
    t->qtdNaJanela++;
  pthread_mutex_unlock(&mutexJanelaDeslizante);
}

void inicializa(int *argc, char* argv[]){
  // alimenta número da porta e tamanho do buffer pelos parâmetros recebidos
  carregaParametros(argc, argv, &porta, &tamMaxMsg, &tamJanela);
  
  t = inicializaTransacao(tamMaxMsg, porta, tamJanela);

  // chamada de função de inicialização para ambiente de testes
  tp_init();
 }

// UTIL
void carregaParametros(int* argc, char** argv, short int* porta, int* tamBuffer, short int* tamJanela){
  char *ultimoCaractere;
  // verifica se programa foi chamado com argumentos corretos
  if (*argc != 4){
    perror("ERRO-> parametros invalidos! Uso: %s [porta] [tam_buffer]\n");
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
    *tamJanela = atoi(argv[3]);
  }
  #ifdef DEBUG
    printf("[DEBUG] Parametros recebidos-> porta: %d tamBuffer: %d tamJanela: %d\n", *porta, *tamBuffer, *tamJanela);
  #endif
}

void limpaBuffer(char *b, int bytes){
  memset(b, 0, bytes);
}

#ifdef STEP
void aguardaEnter(){
  printf("pressione Enter para continuar...");
  while (getchar() != '\n');
}
#endif