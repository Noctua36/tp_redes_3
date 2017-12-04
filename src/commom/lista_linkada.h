#ifndef _LISTA_LINKADA_ 
#define _LISTA_LINKADA_  

#include <stdlib.h>
#include "pacote.h"

typedef struct nodulo {
  pacote *pack;
  struct nodulo *proximo;
} nodulo;

void push(nodulo**, pacote*);
pacote* pop(nodulo**);
void esvaziaLista(nodulo*);

#endif /* _LISTA_LINKADA_  */