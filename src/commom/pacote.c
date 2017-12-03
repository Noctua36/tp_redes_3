#include "pacote.h"
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define DEBUG

// aloca memória e retorna ponteiro para pacote criado
pacote* criaPacoteVazio(){
  int cargaUtilMax = tamMaxMsg - sizeof(uint8_t) - sizeof(uint16_t);
  pacote *p = malloc(sizeof *p);
  p->nomeArquivo = calloc(TAM_NOME_ARQUIVO, sizeof(p->nomeArquivo));
  p->cargaUtil = 0;
  p->dados = calloc(cargaUtilMax, sizeof(p->dados));
  p->mensagemErro = calloc(TAM_MSG_ERRO, sizeof(p->mensagemErro));

  if (p == NULL || p->nomeArquivo == NULL || p->dados == NULL || p->mensagemErro == NULL) {
    perror("Falha ao alocar memoria para pacote.");
    exit(EXIT_FAILURE);
  }

  return p;
}

// libera memória utilizada pelo pacote
void destroiPacote(pacote *p){
  free(p->dados);
  free(p->nomeArquivo);
  free(p->mensagemErro);
  free(p);
  p = NULL;
}

// inicializa pacote
void limpaPacote(pacote *p){
  int cargaUtil = tamMaxMsg - sizeof(p->opcode) - sizeof(p->numBloco);
  p->opcode = (uint8_t)INVALIDO;
  p->codErro = (uint8_t)SEM_ERRO;
  p->cargaUtil = 0;
  memset(p->mensagemErro, 0, TAM_MSG_ERRO);
  memset(p->nomeArquivo, 0, TAM_NOME_ARQUIVO);
  memset(p->dados, 0, cargaUtil);
  p->numBloco = (uint16_t)0;
}

// extrai dados do buffer e os organiza na estrutura do pacote
void montaPacotePelaMensagem(pacote *p, char *b, int tamMsg){
  // inicializa pacote
  limpaPacote(p);

  // identifica tipo de pacote
  carregaOpCode(p, b);

  // monta pacote de acordo com seu tipo
  switch ((opCode)p->opcode){
    case REQ:
      carregaNomeDoArquivo(p, b);
      break;
    case ACK:
      carregaNumeroDoBloco(p, b);
      break;
    case DADOS:
      p->cargaUtil = tamMsg - sizeof p->opcode - sizeof p->numBloco;
      carregaNumeroDoBloco(p, b);
      carregaDados(p, b);
      break;
    case ERRO:
      carregaCodigoErro(p, b);
      carregaMensagemErro(p, b);
      break;
    case FIM:
      break;
    default:
      p->codErro = (uint8_t)COD_ERRO_OP_ILEGAL;
      strcpy(p->mensagemErro, MSG_ERRO_OP_ILEGAL);
  }
}

// cria buffer de pacote a partir dos dados da estrutura do pacote
int montaMensagemPeloPacote(char *b, pacote *p){
  unsigned short posicao = 0;
  // int cargaUtil = tamMaxMsg - sizeof(p->opcode) - sizeof(p->numBloco);
  uint16_t numBloco;
  
  // inicializa buffer
  memset(b, 0, tamMaxMsg);

  memcpy(b, &p->opcode, sizeof p->opcode);
  posicao += sizeof p->opcode;
  switch ((opCode)p->opcode){
    case REQ:
      strcpy(b + posicao, p->nomeArquivo);
      posicao += strlen(p->nomeArquivo);
      break;
    case ACK:
      numBloco = htons(p->numBloco);
      memcpy(b + posicao, &numBloco, sizeof numBloco);
      posicao += sizeof numBloco;
      break;
    case DADOS:
      numBloco = htons(p->numBloco);
      memcpy(b + posicao, &numBloco, sizeof numBloco);
      posicao += sizeof numBloco;
      memcpy(b + posicao, p->dados, p->cargaUtil);
      posicao += p->cargaUtil;
      break;
    case ERRO:
      memcpy(b + posicao, &p->codErro, sizeof p->codErro);
      posicao += sizeof p->codErro;
      strcpy(b + posicao, p->mensagemErro);
      posicao += strlen(p->nomeArquivo);
      break;
    case FIM:
      break;
    case INVALIDO:
      break;
  }
  //TODO: calcular e incluir CRC
  return posicao;
}

// extrai opcode do buffer e carrega no pacote
void carregaOpCode(pacote *p, char *b){
  memcpy(&p->opcode, b, sizeof p->opcode);
}

// extrai nome do arquivo do buffer e carrega no pacote
void carregaNomeDoArquivo(pacote *p, char *b){
  strcpy(p->nomeArquivo, b + sizeof p->opcode);
}

// extrai numero do bloco do buffer e carrega no pacote
void carregaNumeroDoBloco(pacote *p, char *b){
  memcpy(&p->numBloco, b + sizeof p->opcode, sizeof p->numBloco);
  p->numBloco = ntohs(p->numBloco);
}

// extrai dados do buffer e carrega no pacote
void carregaDados(pacote *p, char *b){
  memcpy(p->dados, b + sizeof p->opcode + sizeof p->numBloco, p->cargaUtil);
}

// extrai código de erro do buffer e carrega no pacote
void carregaCodigoErro(pacote *p, char *b){
  memcpy(&p->codErro, b + sizeof p->opcode, sizeof p->codErro);
}

// extrai mensagem de erro do buffer e carrega no pacote
void carregaMensagemErro(pacote *p, char *b){
  strcpy(p->mensagemErro, b + sizeof p->opcode + sizeof p->codErro);
}

void carregaFim(pacote *p, char *b){
  strcpy(p->mensagemErro, b + sizeof p->opcode + sizeof p->codErro);
}

#ifdef DEBUG
void imprimeBuffer(char *b){
  int cargaUtil = tamMaxMsg - sizeof(uint8_t) - sizeof(uint16_t);
  int i;
  printf("[BUFFER]:\n");
  for (i = 0; i < cargaUtil; i++){
    printf("[%d]", b[i]);
  }
  printf("\n");
}

void imprimePacote(pacote *p, int imprimeDados){
  // imprime estrutura
  printf("[PACOTE]:\n");
  printf("opcode: %d\n", p->opcode);
  printf("nomeArquivo: %s\n", p->nomeArquivo);
  printf("cargaUtil: %d\n", p->cargaUtil);
  printf("dados: ");
  if(imprimeDados)
    imprimeBuffer(p->dados);
  else
    printf("<ocultado>");
  printf("\nnumBloco: %d\n", p->numBloco);
  printf("codErro: %d\n", p->codErro);
  printf("mensagemErro: %s\n", p->mensagemErro);
}
#endif