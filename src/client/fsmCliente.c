#include "fsmCliente.h"

// implementação simples da fsm com switch-cases
// recebe estado atual e operação e realiza transição
void transita(int *estado, int *operacao){
  switch (*estado){
    case ESTADO_ENVIA_REQ:
      switch (*operacao){
        case OPERACAO_OK:
          *estado = ESTADO_RECEBE_ARQ;
          break;
        default:
          *estado = ESTADO_ERRO;
      }
    break;

    case ESTADO_ERRO:
      switch (*operacao){
        default:
          *estado = ESTADO_TERMINO;
      }
    break;

    case ESTADO_RECEBE_ARQ:
      switch (*operacao){
        case OPERACAO_IGNORA:
          *estado = ESTADO_RECEBE_ARQ;
          break;
        case OPERACAO_TERMINO:
          *estado = ESTADO_TERMINO;
          break;
        case OPERACAO_OK:
          *estado = ESTADO_ENVIA_ACK;
          break;
        default:
          *estado = ESTADO_ERRO;
      }
    break;

    case ESTADO_ENVIA_ACK:
      switch (*operacao){
        case OPERACAO_OK:
          *estado = ESTADO_RECEBE_ARQ;
          break;
        case OPERACAO_NOK:
          *estado = ESTADO_ERRO;
          break;
      }
    break;

  }
}
