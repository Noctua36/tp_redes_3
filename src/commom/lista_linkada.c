#include "lista_linkada.h"

void push(nodulo *cabeca, pacote *pack) {
  nodulo *atual = cabeca;
  while (atual->proximo != NULL) {
    atual = atual->proximo;
  }
  
  atual->proximo = malloc(sizeof(nodulo));
  atual->proximo->pack = pack;
  atual->proximo->proximo = NULL;
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
