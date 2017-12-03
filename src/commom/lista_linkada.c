#include "lista_linkada.h"

void push(nodulo **cabeca, pacote *p) {
  nodulo *atual = *cabeca;
  // verifica se primeiro
  if (*cabeca == NULL){
    *cabeca = malloc(sizeof(*cabeca));
    (*cabeca)->pack = p;
    (*cabeca)->proximo = NULL;
  }
  else {
    while (atual->proximo != NULL) {
      atual = atual->proximo;
    }
    
    atual->proximo = malloc(sizeof(nodulo));
    atual->proximo->pack = p;
    atual->proximo->proximo = NULL;
  }
}

pacote* pop(nodulo **cabeca){
  pacote *pack = NULL;
  nodulo *proximoNodulo = NULL;

  if (*cabeca != NULL){
    proximoNodulo = (*cabeca)->proximo;
    pack = (*cabeca)->pack;
    free(*cabeca);
    *cabeca = proximoNodulo;
  }
  return pack;
}

void esvaziaLista(nodulo *cabeca){
  while (cabeca) pop(&cabeca);
}
