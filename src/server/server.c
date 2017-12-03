#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include "../commom/tp_socket.h"
#include "../commom/arquivo.h"
#include "../commom/pacote.h"
#include "../commom/transacao.h"
#include "fsmServidor.h"

#define DEBUG
#define IMPRIME_DADOS_DO_PACOTE
#define STEP

#ifdef STEP
void aguardaEnter();
#endif

// protótipo das funções
void inicializa(int*, char**);
void carregaParametros(int*, char**, short int*, int*);
int recebePacoteEsperado(uint8_t);
void limpaBuffer(char*, int);
void estadoStandBy(int*);
void estadoAguardaAck(int*);
void estadoEnvia(int*);
void estadoReseta(int*);
void estadoErro(int*);
void estadoTermino(int*);

short int porta;
int tamMaxMsg;
transacao *t;
struct timeval timeout;      

int main(int argc, char* argv[]){
  printf("Inicio\n");
  //inicializa programa: carrega parâmetros, inicializa variáveis, aloca memória...
  inicializa(&argc, argv);
  
  int estadoAtual = ESTADO_STANDBY;
  int operacao;
  // opera FSM que rege o comportamento do sistema
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
 
  if ((opCode)recebePacoteEsperado((u_int8_t)REQ)){
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
  #endif

  // verifica se arquivo já está aberto
  if (!t->arquivoAberto){
    t->arquivo = abreArquivoParaLeitura(t->recebido->nomeArquivo);
    if(t->arquivo == NULL){
      t->codErro = (uint8_t)COD_ERRO_ARQUIVO_NAO_EXISTE; // TODO: diferenciar erros de permissão de leitura e de existância de arquivo
      *operacao = OPERACAO_NOK;
      return;
    }
    else
      t->arquivoAberto = 1;
  } 

  // verifica se arquivo já chegou ao fim
  if(feof(t->arquivo)){
    *operacao = OPERACAO_TERMINO_ARQ;
    return;
  }
  t->envio = criaPacoteVazio();
    
  // cria pacote para envio
  t->envio->opcode = (uint8_t)DADOS;
  t->envio->numBloco++;
  int bytesArquivo = leBytesDeArquivo(t->envio->dados, t->arquivo, t->cargaUtilPacoteDados);
  t->envio->cargaUtil = bytesArquivo;
  montaMensagemPeloPacote(t->buf, t->envio);

  #ifdef DEBUG
    printf("[DEBUG] Pacote a ser enviado:\n");
    imprimePacote(t->envio, 1);
  #endif
  #ifdef STEP
    aguardaEnter();
  #endif
  // envia parte do arquivo
  int tamMsg = t->envio->cargaUtil + sizeof t->envio->opcode + sizeof t->envio->numBloco;
  int status = tp_sendto(t->socketFd, t->buf, tamMsg, &t->toAddr);
  if (status > 0) {
    *operacao = OPERACAO_OK;
  } 
  else {
    *operacao = OPERACAO_NOK;
  }
  destroiPacote(t->envio);
}

void estadoAguardaAck(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] AGUARDA_ACK\n");
  #endif

  // inicia temporizador de timeout
  if (setsockopt (t->socketFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
    perror("falha ao definir timeout para socket.\n");
    // apenas atualiza contador de timeout, mantém no estado de aguarda ACK
    t->timeoutCount++;
    //TODO: limitar quantidade de timeouts
    return;
  }
  
  *operacao = (opCode)recebePacoteEsperado((uint8_t)ACK);
}

void estadoReseta(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] RESETA\n");
  #endif
  destroiTransacao(t);
  t = criaTransacaoVazia(tamMaxMsg, porta);
  *operacao = OPERACAO_OK;
}

void estadoErro(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] ERRO\n");
  #endif
}

void estadoTermino(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] TERMINO\n");
  #endif
  t->envio = criaPacoteVazio();
  t->envio->opcode = (uint8_t)FIM;
  montaMensagemPeloPacote(t->buf, t->envio);
  #ifdef DEBUG
    printf("[DEBUG] Pacote a ser enviado:\n");
    imprimePacote(t->envio, 0);
  #endif
  #ifdef STEP
    aguardaEnter();
  #endif
  int status = tp_sendto(t->socketFd, t->buf, tamMaxMsg, &t->toAddr);
  if (status > 0) {
    *operacao = OPERACAO_OK;
  }
  //TODO: tratar erro
  destroiPacote(t->envio);
}

int recebePacoteEsperado(uint8_t opCodeEsperado){
  limpaBuffer(t->buf, tamMaxMsg);
  int bytesRecebidos = tp_recvfrom(t->socketFd, t->buf, tamMaxMsg, &t->toAddr);                     
  if (bytesRecebidos == -1){
    perror("ERRO-> Falha no recebimento de mensagem.\n");
    //TODO: avaliar se cria msg de erro na transacao
    return 0;
  }

  montaPacotePelaMensagem(t->recebido, t->buf, bytesRecebidos);
  #ifdef DEBUG
    printf("Recebido mensagem:\n");
    imprimePacote(t->recebido, 0);
  #endif

  if (opCodeEsperado == t->recebido->opcode){
    if (t->recebido->opcode == DADOS){
      //TODO: verificar se verifica CRC apenas para mensagens do tipo DADOS
      int msgIntegra = validaMensagem(t->buf);
      if(!msgIntegra){
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

void inicializa(int *argc, char* argv[]){
  // alimenta número da porta e tamanho do buffer pelos parâmetros recebidos
  carregaParametros(argc, argv, &porta, &tamMaxMsg);
  
  t = criaTransacaoVazia(tamMaxMsg, porta);

  timeout.tv_sec = TIMEOUT;
  timeout.tv_usec = 0;

  // chamada de função de inicialização para ambiente de testes
  tp_init();
 }

// UTIL
void carregaParametros(int* argc, char** argv, short int* porta, int* tamBuffer){
  char *ultimoCaractere;
  // verifica se programa foi chamado com argumentos corretos
  if (*argc != 3){
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
  }
  #ifdef DEBUG
    printf("[DEBUG] Parametros recebidos-> porta: %d tamBuffer: %d\n", *porta, *tamBuffer);
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