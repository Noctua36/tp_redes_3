#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netdb.h>
// #include <netinet/in.h>
// #include <unistd.h>
// #include <arpa/inet.h> //INET_NTOP
// #include <stdbool.h>
#include <time.h>

#define DEBUG

#define TAM_PORTA 6
#define TAM_NOME_ARQUIVO 32
#define TAM_HOST 16



long getTime();
long timeDiff(long, long);
void carregaParametros(int*, char**, char*, char*, char*, int*);

int main(int argc, char* argv[]){
  int tamBuffer;
  // long int bytesRecebidos = 0;
  char porta[TAM_PORTA];
  char arquivo[TAM_NOME_ARQUIVO];
  char host[TAM_HOST];
  // double comeco, duracao;

  // alimenta numero da porta e tamanho do buffer pelos parametros recebidos
  carregaParametros(&argc, argv, host, porta, arquivo, &tamBuffer);

  char* buf;
  
  // aloca memória para buffer
  buf = malloc(sizeof *buf * tamBuffer);
  if (buf == NULL){
    perror("Falha ao alocar memoria para buffer.");
    exit(EXIT_FAILURE);
  }


  exit(EXIT_SUCCESS);
}


// UTIL
void carregaParametros(int* argc, char** argv, char* host, char* porta, char* arquivo, int* tamBuffer){
  #ifdef DEBUG
    printf("[DEBUG] numero de parametros recebidos: %d\n", *argc);
  #endif
  char *ultimoCaractere;
  // verifica se programa foi chamado com argumentos corretos
  if (*argc != 5){
    fprintf(stderr, "ERRO-> parametros invalidos! Uso: %s [host] [porta] [nome_arquivo] [tam_buffer]\n", argv[0]);
    exit(EXIT_FAILURE);
  } 
  else {
    errno = 0;
    *tamBuffer = strtoul(argv[4], &ultimoCaractere, 10);
    if ((errno == ERANGE && (*tamBuffer == LONG_MAX || *tamBuffer == LONG_MIN)) || (errno != 0 && *tamBuffer == 0)){
      perror("strtoul");
      exit(EXIT_FAILURE);
    }
  }
  memset(host, '\0', TAM_HOST);
  strcpy(host, argv[1]);

  memset(porta, '\0', TAM_PORTA);
  strcpy(porta, argv[2]);

  memset(arquivo, '\0', TAM_NOME_ARQUIVO);
  strcpy(arquivo, argv[3]);
  
  #ifdef DEBUG
    printf("[DEBUG] Parametros recebidos-> host: %s porta: %s nome_arquivo: %s tamBuffer: %d\n", host, porta, arquivo, *tamBuffer);
  #endif
}

long getTime(){
    struct timespec tempo;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tempo);
    return tempo.tv_nsec;
}
  
// retorna diferença em nanosegundos
long timeDiff(long start, long end){
  long temp;
  if ((end - start) < 0){
    temp = 1000000000 + end - start;
  } else {
    temp = end - start;
  }
  return temp;
}