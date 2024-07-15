#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <string.h>

#define DEBUG_PARSER 0
#define DEBUG_NODES 1
#define PRINT_TREE_ADDR 0

typedef enum tipoTk {
  INT,
  FLOAT,
  BOOL,
  NONE
} TipoToken;


typedef union {
    int i_val;
    float f_val;
    int b_val;
} Value;

typedef struct valorLexico {
  int linha;
  TipoToken tipo;
  Value valor;
  char *label;
} valorLexico;


typedef struct Nodo {
  valorLexico valor_lexico;
  struct Nodo **filhos;
  int num_filhos;
  TipoToken tipo;
} Nodo;

valorLexico atribui_yylval(char *yytext, TipoToken tipo, int num_lines);

Nodo* cria_nodo(valorLexico valor);
Nodo* cria_nodo_v2(valorLexico valor, TipoToken tipo);
valorLexico cria_call(valorLexico label);
valorLexico cria_valor_lexico(char *label);
valorLexico cria_valor_lexico_v2(char *label, TipoToken tipo);
TipoToken type_infer(Nodo* nodo1, Nodo* nodo2);
void adiciona_filho(Nodo *pai, Nodo *filho);
void print_tree(Nodo *raiz);
void prt_node(void *ptr);
const char* tipoTokenToString(TipoToken tipo);
void exporta(void *arvore);

#endif