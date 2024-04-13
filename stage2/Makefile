IDIR=./include
CC=gcc
CFLAGS=-I$(IDIR) -I$(ODIR) 

ODIR=.
LDIR=.

LIBS=-lm -lfl

_DEPS = parser.tab.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o lex.yy.o parser.tab.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(ODIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: scanner bison etapa2

run: all
	./etapa2

scanner: $(ODIR)/scanner.l
	flex -o $(ODIR)/lex.yy.c $(ODIR)/scanner.l

bison: $(ODIR)/parser.y
	bison -d -o $(ODIR)/parser.tab.c $(ODIR)/parser.y
	$(CC) -c -o $(ODIR)/parser.tab.o $(ODIR)/parser.tab.c $(CFLAGS)

etapa2: $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ $(ODIR)/lex.yy.c $(ODIR)/parser.tab.c $(ODIR)/parser.tab.h

