#include <stdio.h>
#include <string.h>

#define DEBUG 1

typedef enum tipoTk {
  CARACTERE_ESPECIAL,
  OPERADOR_COMPOSTO,
  IDENTIFICADOR,
  LIT_INTEIRO,
  LIT_FLOAT,
  LIT_BOOL,
  LIT_CHAR,
  LIT_STRING,
  NONE
} TipoToken;

typedef struct valorLexico {
  int linha;
  TipoToken tipo;
  char *valorToken;
  char *label;
} valorLexico;


typedef struct Nodo {
  valorLexico valor_lexico;
  struct Nodo **filhos;
  int num_filhos;
} Nodo;

valorLexico atribui_yylval(char *yytext, TipoToken tipo, int num_lines);

Nodo* cria_nodo(valorLexico valor);
valorLexico cria_valor_lexico(char* label);
void adiciona_filho(Nodo *pai, Nodo *filho);
void print_tree(Nodo *raiz);
void exporta(void *arvore);