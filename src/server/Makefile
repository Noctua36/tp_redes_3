CC = gcc
INC_DIR = ../commom/
CFLAGS = -Wall -I$(INC_DIR)

DEPS = fsmServidor.h arquivo.h pacote.h transacao.h tp_socket.h
OBJ = server.o fsmServidor.o $(addprefix $(INC_DIR), arquivo.o pacote.o transacao.o tp_socket.o)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: server

server: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean: 
	@rm -f *.o server $(INC_DIR)*.o