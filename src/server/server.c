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

// protótipo das funções
void inicializa(int, char**);
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
int tamMsg;
transacao *t;
struct timeval timeout;      

int main(int argc, char* argv[]){
  // inicializa programa: carrega parâmentros, inicializa variáveis, aloca memória...
  inicializa(argc, argv);
  
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
 
  *operacao = (opCode)recebePacoteEsperado(REQ);

  // if ((opCode)t->recebido->opcode == (u_int8_t)REQ)
  //   *operacao = OPERACAO_REQ_RECEBIDA;
  // else {
  //   t->codErro = (uint8_t)COD_ERRO_OP_ILEGAL;
  //   *operacao = OPERACAO_NOK;
  // }
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

  t->envio = criaPacoteVazio();
    
  // cria pacote para envio
  t->envio->opcode = (uint8_t)DADOS;
  t->envio->numBloco = t->numBloco++;
  leBytesDeArquivo(t->envio->dados, t->arquivo, t->cargaUtilPacoteDados);
  montaMensagemPeloPacote(t->buf, t->envio);

  // envia parte do arquivo
  int status = tp_sendto(t->socketFd, t->buf, tamMsg, &t->toAddr);
  if (status > 0) {
    // verifica se chegou ao fim do arquivo
    if(feof(t->arquivo))
      *operacao = OPERACAO_TERMINO;
    else
      *operacao = OPERACAO_OK;
  } else {
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
  
  *operacao = (opCode)recebePacoteEsperado(ACK);
}

void estadoReseta(int *operacao){
  #ifdef DEBUG
    printf("\n[FSM] RESETA\n");
  #endif
  destroiTransacao(t);
  t = criaTransacaoVazia();
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
  t->envio->opcode = FIM;
  int status = tp_sendto(t->socketFd, t->buf, tamMsg, &t->toAddr);
  if (status > 0) {
    *operacao = OPERACAO_OK;
  }
  destroiPacote(t->envio);
}

int recebePacoteEsperado(uint8_t opCodeEsperado){
  limpaBuffer(t->buf, tamMsg);
  int bytesRecebidos = tp_recvfrom(t->socketFd, t->buf, tamMsg, &t->toAddr);                     
  if (bytesRecebidos == -1){
    perror("ERRO-> Falha no recebimento de mensagem.\n");
    //TODO: avaliar se cria msg de erro na transacao
    return OPERACAO_NOK;
  }

  #ifdef DEBUG
    printf("Recebido mensagem:\n");
    imprimePacote(t->recebido);
  #endif
  
  montaMensagemPeloPacote(t->buf, t->recebido);

  if (opCodeEsperado == t->recebido->opcode){
    if (t->recebido->opcode == DADOS){
      //TODO: verificar se verifica CRC apenas para mensagens do tipo DADOS
      int msgIntegra = validaMensagem(t->buf);
      if(!msgIntegra){
        #ifdef DEBUG
          printf("Mensagem corrompida recebida!\n");
        #endif
        // mensagem corrompida, apenas ignora, mantendo a FSM no mesmo estado
        return OPERACAO_NOK;
      }  
    }
    else
      return OPERACAO_OK;
  }
  return OPERACAO_NOK;
}

void inicializa(int argc, char* argv[]){
  t = criaTransacaoVazia();

  // alimenta número da porta e tamanho do buffer pelos parâmetros recebidos
  carregaParametros(&argc, argv, &porta, &tamMsg);
  
  // aloca memória para buffer
  t->buf = malloc(sizeof *t->buf * tamMsg);
  if (t->buf == NULL){
    perror("Falha ao alocar memoria para buffer.");
    exit(EXIT_FAILURE);
  }
 
  timeout.tv_sec = TIMEOUT;
  timeout.tv_usec = 0;

  // chamada de função de inicialização para ambiente de testes
  tp_init();
 
  // cria socket e armazena o respectivo file descriptor
  t->socketFd = tp_socket(porta);

  t->recebido = criaPacoteVazio();

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

void limpaBuffer(char *b, int bytes){
  memset(b, 0, bytes);
}