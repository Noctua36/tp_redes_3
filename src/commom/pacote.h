#ifndef _PACOTE_H_ 
#define _PACOTE_H_ 

// #define TAM_CARGA_PACOTE 512
// #define TAM_PACOTE 515
#define TAM_NOME_ARQUIVO 32
#define TAM_MSG_ERRO 32

#define DEBUG
// declara enumeração de códigos de pacotes
//TODO: atualizar tamanho do opcode. Atual 4 bytes pela enumeração
// - REQ - requisição de arquivo. Formato: 
// +-----+---~~---+---+
// | opc |filename| 0 |
// +-----+---~~---+---+
//  2bytes n bytes
// opc - OpCode = REQ
// filename = nome do arquivo requisitado
// 
// - DADOS - dados do arquivo solicitado. Formato: 
// +-----+--------+-------+
// | opc | #bloco | DADOS |
// +-----+--------+-------+
//  2bytes 2bytes   512bytes
// opc - OpCode = DADOS
// #bloco = número do bloco
// DADOS = dados do arquivo a enviado
// 
// - ACK - confirmação de recebimento. Formato: 
// +-----+--------+
// | opc | #bloco |
// +-----+--------+
//  2bytes 2bytes  
// opc - OpCode = ACK
// #bloco = número do bloco
// 
// - ERRO - erro. Formato: 
// +-----+---------+------------------+---+
// | opc | ErrCode | Mensagem de erro | 0 |
// +-----+---------+------------------+---+
//  2bytes 1byte     nbytes
// opc - OpCode = ERRO
// ErrCode = código de erro
// Mensagem de erro
typedef enum opCode { INVALIDO, REQ, DADOS, ACK, ERRO } opCode;

// declara tipos de erros (códigos e mensagens)
// 0    Sem erro
// 1    Não definido, verifique mensagem de erro.
// 2    Arquivo não encontrado.
// 3    Violação de acesso.
// 4    Operação ilegal.
typedef enum codigoErro { SEM_ERRO, COD_ERRO_DESCONHECIDO, COD_ERRO_ARQUIVO_NAO_EXISTE, COD_ERRO_VIOLACAO_ACESSO, COD_ERRO_OP_ILEGAL } codigoErro;
#define MSG_ERRO_DESCONHECIDO "Erro no servidor."
#define MSG_ERRO_ARQUIVO_NAO_EXISTE "Arquivo não encontrado."
#define MSG_ERRO_VIOLACAO_ACESSO "Violação de acesso." 
#define MSG_ERRO_OP_ILEGAL "Operação ilegal."

// mtu deve ser definido no programa principal
extern int mtu;

// estrutura para armazenar dados do pacote
typedef struct pacote {
    opCode opcode;
    char* nomeArquivo;
    char* dados;
    unsigned short numBloco;
    codigoErro codErro;
    char* mensagemErro;
} pacote;

pacote* criaPacoteVazio();
void destroiPacote(pacote*);
void limpaPacote(pacote*);

void carregaOpCode(pacote*, char*);
void carregaNomeDoArquivo(pacote*, char*);
void carregaDados(pacote*, char*);
void carregaNumeroDoBloco(pacote*, char*);
void carregaCodigoErro(pacote*, char*);
void carregaMensagemErro(pacote*, char*);

void montaPacotePeloBuffer(pacote*, char*);
void montaBufferPeloPacote(char*, pacote*);

#ifdef DEBUG
void imprimeBuffer(char*);
void imprimePacote(pacote*);
#endif

#endif /* _PACOTE_H_ */