#ifndef _FSM_CLIENTE_H_ 
#define _FSM_CLIENTE_H_ 

// define estados da fsm
#define ESTADO_ENVIA_REQ      1        
#define ESTADO_RECEBE_ARQ     2
#define ESTADO_ERRO           3 
#define ESTADO_ENVIA_ACK      4
#define ESTADO_TERMINO        5

// define operações
#define OPERACAO_OK           1
#define OPERACAO_NOK          2
#define OPERACAO_TERMINO  3
#define OPERACAO_IGNORA       4

void transita(int *estado, int *operacao);

#endif /* _FSM_CLIENTE_H_ */