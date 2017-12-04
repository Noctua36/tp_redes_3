CC = gcc
INC_DIR = ../commom/
CFLAGS = -Wall -I$(INC_DIR) -lpthread -g

DEPS = fsmServidor.h arquivo.h pacote.h transacao.h crc.h tp_socket.h lista_linkada.h
OBJ = server.o fsmServidor.o $(addprefix $(INC_DIR), arquivo.o pacote.o transacao.o crc.o tp_socket.o lista_linkada.o)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: server

server: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean: 
	@rm -f *.o server $(INC_DIR)*.o