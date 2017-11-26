#include "arquivo.h"

FILE* abreArquivoParaLeitura(char *nomeArquivo){
    return fopen(nomeArquivo, "r");
}

FILE* abreArquivoParaEscrita(char *nomeArquivo){
    return fopen(nomeArquivo, "a+"); // abre no modo appending se arquivo existe
}

// retorna quantidade de bytes lidos
int leBytesDeArquivo(char *buf, FILE* arquivo, unsigned short numBytes){
    return fread(buf, 1, numBytes, arquivo);
}

// retorna quantidade de bytes escritos
int escreveBytesEmArquivo(char *buf, FILE* arquivo, unsigned short numBytes){
    return fwrite(buf, 1, numBytes, arquivo);
}

int verificaSeArquivoExiste(char *nomeArquivo){
    return access(nomeArquivo, F_OK) != -1;
}

void fechaArquivo(FILE* arquivo){
    fclose(arquivo);
    arquivo = NULL;
}

int excluiArquivo(char *nomeArquivo){
    return remove(nomeArquivo);
}