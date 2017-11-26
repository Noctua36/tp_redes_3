#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h> //INET_NTOP
#include <stdbool.h>

//para marcar o tempo
#include <time.h>

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

#define DEBUG

void carregaParametros(int*, char**, char*, char*, char*, long int*);
void tentaConectar(struct addrinfo *res, int *sockfd);

int main(int argc, char* argv[]){
  long int tamBuffer;
  long int bytesRecebidos = 0;
  char porta[6];
  char arquivo[16];
  char host[16];
  double comeco, duracao;

  // alimenta numero da porta e tamanho do buffer pelos parametros recebidos
  carregaParametros(&argc, argv, host, porta, arquivo, &tamBuffer);

  char* buffer;
  
  struct addrinfo hints, *resultados, *cliAddr;
  int sockCliFd;
  socklen_t sinSize;
  
  memset(&hints, 0, sizeof hints); // certifica que struct esta zerada
  hints.ai_family = AF_UNSPEC; // não importa se ipv4 ou ipv6
  hints.ai_socktype = SOCK_STREAM; // com conexão

  // preenche estruturas addrinfo
  // getaddrinfo retorna lista linkada de endereços.
  int errNo = getaddrinfo(host, porta, &hints, &resultados);
  if (errNo != 0){
    perror("Falha ao montar estruturas para socket");
    exit(EXIT_FAILURE);
  };

  // programa é finalizado por tentaConectar caso não seja possível abrir conexão
  tentaConectar(resultados, &sockCliFd);

  // se conexão foi aberta em um dos endereços, não é necessário manter
  // libera variáveis não utilizadas
  freeaddrinfo(resultados);
  //freeaddrinfo(&hints);

  // aloca memória para buffer
  buffer = malloc(sizeof *buffer * tamBuffer);
  if (buffer == NULL){
    perror("Falha ao alocar memoria para buffer.");
    exit(EXIT_FAILURE);
  }

  // envia solicitacao de arquivo para servidor
  if (send(sockCliFd, arquivo, sizeof(arquivo), 0) == -1){
    close(sockCliFd);
    perror("Erro ao enviar nome do arquivo para servidor");
    exit(EXIT_FAILURE);
  }

  bytesRecebidos = recv(sockCliFd, buffer, tamBuffer, 0);
  // verifica resposta
  if(strcmp(buffer, "NOK") == 0){
    printf("Arquivo nao encontrado\n");
  }
  else {
    printf("OK\n");
  }

  //inicia marcacao de tempo
  // comeco = getTime();
  // printf("iniciando contagem de tempo\n");

  // FILE * file;
  // file = fopen(arquivo, "w");
  // if (file != NULL){
    
  //   unsigned long int bytesRecebidosTotal = 0;
  //   while ((bytesRecebidos = recv(sockCliFd, buffer, tamBuffer, 0)) >= 0){
  //     #ifdef DEBUG
  //       printf("Recebido: ");
  //       for (int j = 0; j < tamBuffer; j++){
  //         printf("[%d]", buffer[j]);
  //       }
  //       printf("\n");
  //     #endif
  //     bytesRecebidosTotal += bytesRecebidos;
  //     if (fwrite(buffer, 1, tamBuffer, file) < tamBuffer){
  //       close(sockCliFd);
  //       fclose(file);
  //       perror("Erro ao escrever em arquivo");
  //       exit(EXIT_FAILURE);
  //     }
  //   }
  //   //termina marcacao de tempo
  //   duracao = timeDiff(comeco, getTime());
  //   if (bytesRecebidosTotal > 0){
  //     fclose(file); // apenas salva arquivo se foi recebido      
  //     printf("Sucesso.\n");
  //     printf("Buffer = %ld byte(s), %10.2f kbps (%ld bytes em %.6f s)\n", bytesRecebidosTotal, bytesRecebidosTotal / (duracao / 1000000000) * 8 / 1000, bytesRecebidosTotal, duracao / 1000000000);
  //   }
  //   else {
  //     printf("Falha, arquivo não encontrado!\n");
  //   }
  // }
  // else {
  //   printf("Erro ao abrir %s para escrita\n", arquivo);
  //   perror("fopen");
  //   close(sockCliFd);
  //   exit (EXIT_FAILURE);
  // }

  exit(EXIT_SUCCESS);
}


// UTIL
void carregaParametros(int* argc, char** argv, char* host, char* porta, char* arquivo, long int* tamBuffer){
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
  memset(host, '\0', sizeof(host));
  strcpy(host, argv[1]);

  memset(porta, '\0', sizeof(porta));
  strcpy(porta, argv[2]);

  memset(arquivo, '\0', sizeof(arquivo));
  strcpy(arquivo, argv[3]);
  
  #ifdef DEBUG
    printf("[DEBUG] Parametros recebidos-> host: %s porta: %s nome_arquivo: %s tamBuffer: %ld\n", host, porta, arquivo, *tamBuffer);
  #endif
}

// recebe lista linkada de addrinfo e retorna file descriptor de socket com bind realizado com sucesso no primeiro endereço possível
void tentaConectar(struct addrinfo *res, int *sockfd){
  struct addrinfo *p;
  bool sucesso = false;
  for (p = res; p != NULL; p = res->ai_next){
    #ifdef DEBUG  
      printf("tentando bind, aguarde...\n");
    #endif
    // verifica tentatica de criação de socket
    *sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (*sockfd == -1){
      perror("server: socket");
      continue;
    }
    // tenta conectar
    if (connect(*sockfd, p->ai_addr, p->ai_addrlen) == -1){
      perror("connect");
      close(*sockfd);
      continue;
    } 
    else {
      sucesso = true;
      printf("Conectado com sucesso\n");
      break;
    }  
  
    if (!sucesso){
      perror("Não foi possível conectar não possível\n");
      exit(EXIT_FAILURE);
    }
  }
}
