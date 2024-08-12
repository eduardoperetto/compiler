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

_OBJ = main.o lex.yy.o parser.tab.o tree.o hash_table.o assembly.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(ODIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: scanner bison etapa6

run: all
	./etapa6 < input.txt

scanner: $(ODIR)/scanner.l
	flex -o $(ODIR)/lex.yy.c $(ODIR)/scanner.l
	
BISON_FLAGS=-r all -v --report-file bison_output -Wcounterexamples
bison: $(ODIR)/parser.y
	bison -d $(BISON_FLAGS) -o $(ODIR)/parser.tab.c $(ODIR)/parser.y
	$(CC) -c -o $(ODIR)/parser.tab.o $(ODIR)/parser.tab.c $(CFLAGS)
	$(CC) -c -o $(ODIR)/lex.yy.o $(ODIR)/lex.yy.c $(LIBS)

etapa6: $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

debug: etapa6
	gdb --args ./etapa6

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ $(ODIR)/lex.yy.c $(ODIR)/parser.tab.c $(ODIR)/parser.tab.h ./etapa6 ./bison_output

tar:
	@echo "REMEMBER TO SET DEBUG TO 0"
	rm -f ./etapa6.tgz
	tar cvzf etapa6.tgz .

asm:
	@$(MAKE) -s all
	@$(SH) ./etapa6 < input.txt > output.s

.PHONY: exec
exec:
	@$(MAKE) -s asm
	gcc output.s -O0 -o exec

asm_c:
	gcc -O0 -S input_c.c

c_exec:
	@$(MAKE) -s asm_c
	gcc input_c.s -O0 -o execc

cmp:
	@$(MAKE) -s c_exec
	@RET_EXEC_C=`./execc; echo $$?`; \
	$(MAKE) -s exec; \
	RET_EXEC=`./exec; echo $$?`; \
	echo "RET_EXEC_C = $$RET_EXEC_C"; \
	echo "RET_EXEC = $$RET_EXEC"; \
	if [ $$RET_EXEC -eq $$RET_EXEC_C ]; then \
		echo "Return codes are the same"; \
	else \
		echo "Return codes differ"; \
	fi
