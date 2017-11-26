CC = gcc
CFLAGS = -Wall -I.
DEPS = fsmServidor.h fsmCliente.h arquivo.h pacote.h transacao.h tp_socket.h
OBJ = server.o client.o fsmServidor.o fsmCliente.o arquivo.o pacote.o transacao.o tp_socket.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: server client

server: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

client: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

testes:
	gcc -o testePacote testePacote.c pacote.c  
	gcc -o testeArquivo testeArquivo.c arquivo.c  

clean: 
	@rm -f *.o server testePacote testeArquivo
