#include "pacote.h"
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define DEBUG

// aloca memória e retorna ponteiro para pacote criado
pacote* criaPacoteVazio(){
    pacote *p = malloc(sizeof *p);
    p->nomeArquivo = malloc(TAM_NOME_ARQUIVO * sizeof(char));
    p->dados = malloc(TAM_CARGA_PACOTE * sizeof(char));
    p->mensagemErro = malloc(TAM_MSG_ERRO * sizeof(char));

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
    p->opcode = INVALIDO;
    p->codErro = SEM_ERRO;
    memset(p->mensagemErro, 0, TAM_MSG_ERRO);
    memset(p->nomeArquivo, 0, TAM_NOME_ARQUIVO);
    memset(p->dados, 0, TAM_CARGA_PACOTE);
    p-> numBloco = 0;
}

// extrai dados do buffer e os organiza na estrutura do pacote
void montaPacotePeloBuffer(pacote *p, char *b){
    // identifica tipo de pacote
    carregaOpCode(p, b);

    // inicializa pacote
    p->codErro = SEM_ERRO;
    memset(p->mensagemErro, 0, TAM_MSG_ERRO);
    memset(p->nomeArquivo, 0, TAM_NOME_ARQUIVO);
    memset(p->dados, 0, TAM_CARGA_PACOTE);
    p-> numBloco = 0;

    p->opcode = ntohs(p->opcode);
    // monta pacote de acordo com seu tipo
    switch (p->opcode){
        case REQ:
            carregaNomeDoArquivo(p, b);
            break;
        case ACK:
            carregaNumeroDoBloco(p, b);
            break;
        case DADOS:
            carregaDados(p, b);
            break;
        case ERRO:
            carregaCodigoErro(p, b);
            carregaMensagemErro(p, b);
            break;
        default:
            p->codErro = COD_ERRO_OP_ILEGAL;
            strcpy(p->mensagemErro, MSG_ERRO_OP_ILEGAL);
    }

}

// cria buffer de pacote a partir dos dados da estrutura do pacote
void montaBufferPeloPacote(char *b, pacote *p){
    unsigned short posicao = 0;

    // inicializa buffer
    memset(b, 0, TAM_PACOTE);

    memcpy(b, &p->opcode, sizeof p->opcode);
    posicao += sizeof p->opcode;

    switch (p->opcode){
        case REQ:
            strcpy(b + posicao, p->nomeArquivo);
            break;
        case ACK:
            p->numBloco = ntohs(p->numBloco);
            memcpy(b + posicao, &p->numBloco, sizeof p->numBloco);
            break;
        case DADOS:
            p->numBloco = ntohs(p->numBloco);
            memcpy(b + posicao, &p->numBloco, sizeof p->numBloco);
            posicao += sizeof p->numBloco;
            memcpy(b + posicao, &p->dados, TAM_CARGA_PACOTE);
            break;
        case ERRO:
            p->codErro = ntohs(p->codErro);
            memcpy(b + posicao, &p->codErro, sizeof p->codErro);
            posicao += sizeof p->codErro;
            strcpy(b + posicao, p->mensagemErro);
            break;
        case INVALIDO:
            break;
    }
    p->opcode = htons(p->opcode);
}

// extrai opcode do buffer e carreda no pacote
void carregaOpCode(pacote *p, char *b){
    // opCode é sempre o primeiro byte
    memcpy(&p->opcode, b, sizeof p->opcode);
    p->opcode = ntohs(p->opcode);
}

// extrai nome do arquivo do buffer e carreda no pacote
void carregaNomeDoArquivo(pacote *p, char *b){
    strcpy(p->nomeArquivo, b + sizeof p->opcode);
}

// extrai numero do bloco do buffer e carreda no pacote
void carregaNumeroDoBloco(pacote *p, char *b){
    memcpy(&p->numBloco, b + sizeof p->opcode, sizeof p->numBloco);
    p->numBloco = ntohs(p->numBloco);
}

// extrai dados do buffer e carreda no pacote
void carregaDados(pacote *p, char *b){
    memcpy(&p->dados, b + sizeof p->opcode + sizeof p->numBloco, TAM_CARGA_PACOTE);
    p->numBloco = ntohs(p->numBloco);
}

// extrai código de erro do buffer e carreda no pacote
void carregaCodigoErro(pacote *p, char *b){
    memcpy(&p->codErro, b + sizeof p->opcode, sizeof p->codErro);
    p->codErro = ntohs(p->codErro);
}

// extrai mensagem de erro do buffer e carreda no pacote
void carregaMensagemErro(pacote *p, char *b){
    strcpy(p->mensagemErro, b + sizeof p->opcode + sizeof p->codErro);
}

#ifdef DEBUG
void imprimeBuffer(char *b){
    int i;
    printf("BUFFER>\n");
    for (i = 0; i < TAM_PACOTE;i++){
        printf("[%d]", b[i]);
    }
    printf("\n");

}

void imprimePacote(pacote *p){
    // imprime estrutura
    printf("PACOTE>\n");
    printf("opcode: %d\n", p->opcode);
    printf("nomeArquivo: %s\n", p->nomeArquivo);
    printf("dados: %s\n", p->dados);
    printf("numBloco: %d\n", p->numBloco);
    printf("codErro: %d\n", p->codErro);
    printf("mensagemErro: %s\n", p->mensagemErro);
}
#endif