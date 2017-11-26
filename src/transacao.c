#include "transacao.h"

transacao* criaTransacaoVazia(){
    transacao *t = malloc(sizeof *t);

    t->timeoutCount = 0;
    t->timedout = 0;
    t->terminada = 0;
    t->arquivoAberto = 0;
    t->posicaoArquivo = 0;
    t->tamBufferArquivo = 0;
    t->numBloco = 0;
    t->codErro = SEM_ERRO;

    t->mensagemErro = malloc(TAM_MSG_ERRO * sizeof(char));
    // t->bufferArquivo = malloc(TAM_CARGA_PACOTE * sizeof(char));
    t->arquivo = malloc(TAM_CARGA_PACOTE * sizeof(char));

    return t;
}