CC = gcc
INC_DIR = ../commom/
CFLAGS = -Wall -I$(INC_DIR)

DEPS = arquivo.h pacote.h
OBJ = $(addprefix $(INC_DIR), arquivo.o pacote.o)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: testePacote testeArquivo

testePacote: testePacote.o $(OBJ) 
	gcc -o $@ $^ $(CFLAGS)

testeArquivo: testeArquivo.o $(OBJ) 
	gcc -o $@ $^ $(CFLAGS)

clean: 
	@rm -f *.o testePacote testeArquivo $(INC_DIR)*.o