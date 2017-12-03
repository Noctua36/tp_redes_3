#include "fsmServidor.h"

// implementação simples da fsm com switch-cases
// recebe estado atual e operação e realiza transição
void transita(int *estado, int *operacao){
  switch (*estado){
    case ESTADO_STANDBY:
      switch (*operacao){
        case OPERACAO_REQ_RECEBIDA:
          *estado = ESTADO_ENVIA;
          break;
        case OPERACAO_ABANDONA:
          *estado = ESTADO_RESETA;
          break;
        case OPERACAO_IGNORA:
          *estado = ESTADO_STANDBY;
          break;
        default:
          *estado = ESTADO_ERRO;
      }
    break;

    case ESTADO_ERRO:
      switch (*operacao){
        default:
          *estado = ESTADO_RESETA;
      }
    break;

    case ESTADO_RESETA:
      switch (*operacao){
        default:
          *estado = ESTADO_STANDBY;
      }
    break;

    case ESTADO_ENVIA:
      switch (*operacao){
        case OPERACAO_NOK:
          *estado = ESTADO_ERRO;
          break;
        case OPERACAO_ABANDONA:
          *estado = ESTADO_RESETA;
          break;
        case OPERACAO_OK:
          *estado = ESTADO_AGUARDA_ACK;
          break;
        case OPERACAO_TERMINO:
          *estado = ESTADO_TERMINO;
          break;
        default:
          *estado = ESTADO_ERRO;  
      }
    break;

    case ESTADO_AGUARDA_ACK:
      switch (*operacao){
        case OPERACAO_OK:
          *estado = ESTADO_ENVIA;
          break;
        case OPERACAO_NOK:
          *estado = ESTADO_AGUARDA_ACK;
          break;
        case OPERACAO_ABANDONA:
          *estado = ESTADO_RESETA;
          break;
        default:
          *estado = ESTADO_ERRO;
      }
    break;

    case ESTADO_TERMINO:
      switch (*operacao){
        case OPERACAO_NOK:
          *estado = ESTADO_ERRO;
          break;
        case OPERACAO_OK:
          *estado = ESTADO_RESETA;
          break;
        default:
          *estado = ESTADO_ERRO;
      }
    break;
  }
}
