# Eduardo Raupp Peretto (00313439) e João Pedro Linz

.PHONY: scanner

IDIR=./include
CC=gcc
CFLAGS=-I$(IDIR) -I$(ODIR) 

ODIR=.
LDIR=.

LIBS=-lm -lfl

_DEPS = parser.tab.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o lex.yy.o parser.tab.o tree.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(ODIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: scanner bison etapa3

run: all
	./etapa3 < input.txt

scanner: $(ODIR)/scanner.l
	flex -o $(ODIR)/lex.yy.c $(ODIR)/scanner.l
	
BISON_FLAGS=-r all -v --report-file bison_output -Wcounterexamples
bison: $(ODIR)/parser.y
	bison -d $(BISON_FLAGS) -o $(ODIR)/parser.tab.c $(ODIR)/parser.y
	$(CC) -c -o $(ODIR)/parser.tab.o $(ODIR)/parser.tab.c $(CFLAGS)
	$(CC) -c -o $(ODIR)/lex.yy.o $(ODIR)/lex.yy.c $(LIBS)

etapa3: $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

debug: etapa3
	gdb --args ./etapa3

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ $(ODIR)/lex.yy.c $(ODIR)/parser.tab.c $(ODIR)/parser.tab.h ./etapa3 ./bison_output

tar:
	@echo "REMEMBER TO SET DEBUG TO 0"
	rm -f ./etapa3.tgz
	tar cvzf etapa3.tgz .

dot:
	rm -f saida.dot
	./etapa3 | ./output2dot.sh < input.txt > saida.dot