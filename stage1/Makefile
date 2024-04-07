IDIR=./include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=./
LDIR=./

LIBS=-lm -lfl

_DEPS = tokens.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o lex.yy.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: scanner etapa1

run: all
	./etapa1

scanner: scanner.l
	flex scanner.l
	$(CC) -c -o lex.yy.o lex.yy.c $(CFLAGS)

etapa1: $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ lex.yy.c
