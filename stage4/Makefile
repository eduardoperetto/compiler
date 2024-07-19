# Eduardo Raupp Peretto (00313439)

.PHONY: scanner

IDIR=./include
CC=gcc
CFLAGS=-I$(IDIR) -I$(ODIR) 

ODIR=.
LDIR=.

LIBS=-lm -lfl

_DEPS = parser.tab.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o lex.yy.o parser.tab.o tree.o hash_table.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(ODIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: scanner bison etapa4

run: all
	./etapa4 < input.txt

scanner: $(ODIR)/scanner.l
	flex -o $(ODIR)/lex.yy.c $(ODIR)/scanner.l
	
BISON_FLAGS=-r all -v --report-file bison_output -Wcounterexamples
bison: $(ODIR)/parser.y
	bison -d $(BISON_FLAGS) -o $(ODIR)/parser.tab.c $(ODIR)/parser.y
	$(CC) -c -o $(ODIR)/parser.tab.o $(ODIR)/parser.tab.c $(CFLAGS)
	$(CC) -c -o $(ODIR)/lex.yy.o $(ODIR)/lex.yy.c $(LIBS)

etapa4: $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

debug: etapa4
	gdb --args ./etapa4

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ $(ODIR)/lex.yy.c $(ODIR)/parser.tab.c $(ODIR)/parser.tab.h ./etapa4 ./bison_output

tar:
	@echo "REMEMBER TO SET DEBUG TO 0"
	rm -f ./etapa4.tgz
	tar cvzf etapa4.tgz .

dot:
	rm -f saida.dot
	@$(MAKE) -s run
	./etapa4 < input.txt | ./output2dot.sh > saida.dot
