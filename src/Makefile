CC = gcc
CFLAGS = -Wall -I.
DEPS = fsmServidor.h arquivo.h pacote.h transacao.h tp_socket.h
OBJ = server.o fsmServidor.o arquivo.o pacote.o transacao.o tp_socket.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

testes:
	gcc -o testePacote testePacote.c pacote.c  
	gcc -o testeArquivo testeArquivo.c arquivo.c  

clean: 
	@rm -f *.o server testePacote testeArquivo
