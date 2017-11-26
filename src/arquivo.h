#ifndef _ARQUIVO_H_ 
#define _ARQUIVO_H_ 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

FILE* abreArquivoParaLeitura(char *nomeArquivo);

FILE* abreArquivoParaEscrita(char*);
int leBytesDeArquivo(char*, FILE* , unsigned short);
int escreveBytesEmArquivo(char*, FILE* , unsigned short);
int verificaSeArquivoExiste(char*);
void fechaArquivo(FILE*);
int excluiArquivo(char*);

#endif /* _ARQUIVO_H_ */