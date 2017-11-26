CC = gcc
INC_DIR = ../commom/
CFLAGS = -Wall -I$(INC_DIR) -g

DEPS = fsmCliente.h arquivo.h pacote.h transacao.h tp_socket.h
OBJ = client.o fsmCliente.o $(addprefix $(INC_DIR), arquivo.o pacote.o transacao.o tp_socket.o)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: client

client: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean: 
	@rm -f *.o client $(INC_DIR)*.o