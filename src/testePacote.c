#include <stdio.h>
#include <stdlib.h>
#include "pacote.h"
#include <string.h>
#include <netinet/in.h>

int main(){
    pacote *p, *p2;
    p = criaPacoteVazio();
    p2 = criaPacoteVazio();
    char *b;
    b = malloc(TAM_PACOTE * sizeof (char));
    
    // teste pacote REQ
    printf("\nTeste REQ\n");
    limpaPacote(p);
    p->opcode = REQ;

    char *nomeArquivo = "arquivo.txt";
    strcpy(p->nomeArquivo, nomeArquivo);
    imprimePacote(p);
    montaBufferPeloPacote(b, p);
    //imprimeBuffer(b);
    limpaPacote(p2);
    montaPacotePeloBuffer(p2, b);
    imprimePacote(p2);

    // teste pacote ACK
    printf("\nTeste ACK\n");
    limpaPacote(p);
    p->opcode = ACK;
    p->numBloco = 12;
    imprimePacote(p);
    montaBufferPeloPacote(b, p);
    //imprimeBuffer(b);
    limpaPacote(p2);
    montaPacotePeloBuffer(p2, b);
    imprimePacote(p2);

    // teste pacote DADOS
    printf("\nTeste DADOS\n");
    limpaPacote(p);
    p->opcode = DADOS;
    for (int i = 0; i < TAM_CARGA_PACOTE; i++){
        p->dados[i] = 'a';
    }
    p->numBloco = 9;
    imprimePacote(p);
    montaBufferPeloPacote(b, p);
    //imprimeBuffer(b);
    limpaPacote(p2);
    montaPacotePeloBuffer(p2, b);
    imprimePacote(p2);

    // teste pacote ERRO
    printf("\nTeste ERRO\n");
    limpaPacote(p);
    p->opcode = ERRO;
    p->codErro = COD_ERRO_ARQUIVO_NAO_EXISTE;
    char *erro = MSG_ERRO_ARQUIVO_NAO_EXISTE;
    strcpy(p->mensagemErro, erro);
    imprimePacote(p);
    montaBufferPeloPacote(b, p);
    //imprimeBuffer(b);
    limpaPacote(p2);
    montaPacotePeloBuffer(p2, b);
    imprimePacote(p2);

    return 0;
}