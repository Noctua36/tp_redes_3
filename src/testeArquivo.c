#include <stdio.h>
#include <stdlib.h>
#include "arquivo.h"
#include <string.h>

#define TAM_MAX 32
#define PI "3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148086513282306647093844609550582231725359408128481117450284102701938521105559644622948954930381964428810975665933446128475648233786783165271201909145648566923460348610454326648213393607260249141273"

void imprimeBuf(char *b){
    printf("%p\n",b);
    for (int i = 0; i < TAM_MAX; ++i){
        printf("[%c]", (b[i] == 0) ? ' ' : b[i]);
    }
    printf("\n");
}

int main(){
    int status, pos = 0;
    char *b, *pi;
    b = malloc(sizeof *b * TAM_MAX);

    pi = PI;
    memset(b, 0, TAM_MAX);
    
    printf("\nTeste biblioteca arquivo \n");
    char *nomeArquivo = "teste.txt";
    
    // remove arquivo se existir
    excluiArquivo(nomeArquivo);

    // TESTE VERIFICAÇÃO DE EXISTÊNCIA QUANDO NÃO EXISTE
    // arquivo não deve existir a princípio
    status = verificaSeArquivoExiste(nomeArquivo);
    if (status) printf("[NOK] > Arquivo existe\n");
    else printf("[OK]\n");

    // TESTE ABERTURA PARA ESCRITA
    FILE *arq = abreArquivoParaEscrita(nomeArquivo);
    if (arq == NULL) printf("[NOK] > Nao foi possível abrir arquivo para escrita\n");
    else printf("[OK]\n");

    // TESTE VERIFICAÇÃO DE EXISTÊNCIA QUANDO EXISTE
    // agora arquivo deve existir
    status = verificaSeArquivoExiste(nomeArquivo);
    if (status) printf("[OK]\n");
    else printf("[NOK] > Arquivo nao existe\n");

    // TESTE ESCREVE 4 CARACTERES, 2 A 2
    // remove arquivo se existir
    excluiArquivo(nomeArquivo);
    arq = abreArquivoParaEscrita(nomeArquivo);
    status = escreveBytesEmArquivo(pi, arq, 2);
    fechaArquivo(arq); // para salvar
    if (status != 2) printf("[NOK] > quantidade de bytes escritos nao conforme\n");
    else printf("[OK]\n");
    arq = abreArquivoParaEscrita(nomeArquivo);
    status = escreveBytesEmArquivo(pi + 2, arq, 2);
    fechaArquivo(arq); // para salvar
    if (status != 2) printf("[NOK] > quantidade de bytes escritos nao conforme\n");
    else printf("[OK]\n");

    // TESTE ESCREVE TODOS CARACTERES
    excluiArquivo(nomeArquivo);
    arq = abreArquivoParaEscrita(nomeArquivo);
    status = escreveBytesEmArquivo(pi, arq, 302);
    fechaArquivo(arq); // para salvar
    if (status != 302) printf("[NOK] > quantidade de bytes escritos nao conforme\n");
    else printf("[OK]\n");

    // TESTE LEITURA 16 CARACTERES, PRIMEIRO 2 DEPOIS 14
    memset(b, 0, TAM_MAX);
    arq = abreArquivoParaLeitura(nomeArquivo);
    leBytesDeArquivo(b + pos, arq, 2);
    imprimeBuf(b);
    pos += 2;
    leBytesDeArquivo(b + pos, arq, 14);
    imprimeBuf(b);
    fechaArquivo(arq);

    // TESTE LEITURA 32 CARACTERES
    memset(b, 0, TAM_MAX);
    imprimeBuf(b);
    arq = NULL;
    pos = 0;
    arq = abreArquivoParaLeitura(nomeArquivo);
    leBytesDeArquivo(b + pos, arq, TAM_MAX);
    imprimeBuf(b);

    excluiArquivo(nomeArquivo);
    
    
    return 0;
}