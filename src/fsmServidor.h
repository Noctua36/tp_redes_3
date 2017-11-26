#ifndef _FSM_SERVIDOR_H_ 
#define _FSM_SERVIDOR_H_ 

// define estados da fsm
#define ESTADO_STANDBY        1
#define ESTADO_ENVIA          2
#define ESTADO_ERRO           3 
#define ESTADO_RESETA         4
#define ESTADO_AGUARDA_ACK    5

// define operações
#define OPERACAO_OK           1
#define OPERACAO_NOK          2
#define OPERACAO_ABANDONA     3
#define OPERACAO_REQ_RECEBIDA 4

void transita(int *estado, int *operacao);

#endif /* _FSM_SERVIDOR_H_ */