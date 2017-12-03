CC = gcc
INC_DIR = ../commom/
CFLAGS = -Wall -I$(INC_DIR)

DEPS = arquivo.h pacote.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: testePacote testeArquivo

testePacote: testePacote.o $(addprefix $(INC_DIR), pacote.o)
	gcc -o $@ $^ $(CFLAGS)

testeArquivo: testeArquivo.o $(addprefix $(INC_DIR), arquivo.o)
	gcc -o $@ $^ $(CFLAGS)

clean: 
	@rm -f *.o testePacote testeArquivo $(INC_DIR)*.o