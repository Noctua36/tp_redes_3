#include <stdio.h>
#include <stdlib.h>
#include "../commom/pacote.h"
#include <string.h>
#include <netinet/in.h>

int tamMaxMsg = 1024;

int main(){
    pacote *p, *p2;
    p = criaPacoteVazio();
    p2 = criaPacoteVazio();
    char *b;
    b = malloc(tamMaxMsg * sizeof (char));
    
    // teste pacote REQ
    printf("\nTeste REQ\n");
    limpaPacote(p);
    p->opcode = (uint8_t)REQ;

    char *nomeArquivo = "arquivo.txt";
    strcpy(p->nomeArquivo, nomeArquivo);
    imprimePacote(p, 0);
    montaMensagemPeloPacote(b, p, 1022);
    limpaPacote(p2);
    montaPacotePelaMensagem(p2, b);
    imprimePacote(p2, 0);

    // teste pacote ACK
    printf("\nTeste ACK\n");
    limpaPacote(p);
    p->opcode = (uint8_t)ACK;
    p->numBloco = (uint16_t)12;
    imprimePacote(p, 0);
    montaMensagemPeloPacote(b, p, 1022);
    limpaPacote(p2);
    montaPacotePelaMensagem(p2, b,1021);
    imprimePacote(p2, 0);

    // teste pacote DADOS
    printf("\nTeste DADOS\n");
    limpaPacote(p);
    p->opcode = (uint8_t)DADOS;
    int cargaUtil = tamMaxMsg - sizeof(opCode) - sizeof(unsigned short);
    for (int i = 0; i < cargaUtil; i++){
        p->dados[i] = 'a';
    }
    p->numBloco = (uint16_t)9;
    imprimePacote(p, 1);
    montaMensagemPeloPacote(b, p, 1022);
    limpaPacote(p2);
    montaPacotePelaMensagem(p2, b);
    imprimePacote(p2, 1);

    // teste pacote ERRO
    printf("\nTeste ERRO\n");
    limpaPacote(p);
    p->opcode = (uint8_t)ERRO;
    p->codErro = (uint8_t)COD_ERRO_ARQUIVO_NAO_EXISTE;
    char *erro = MSG_ERRO_ARQUIVO_NAO_EXISTE;
    strcpy(p->mensagemErro, erro);
    imprimePacote(p, 0);
    montaMensagemPeloPacote(b, p);
    limpaPacote(p2);
    montaPacotePelaMensagem(p2, b);
    imprimePacote(p2, 0);

    return 0;
}