#ifndef TREE_H
#define TREE_H

#include "iloc.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define DEBUG_PARSER 0
#define DEBUG_NODES 0
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
  int table_local_addr;
  ilocCode* iloc_code;
  char* temp_reg;
  char* temp_reg_false;
  TipoToken tipo;
} Nodo;

typedef struct StoredIdentifier {
  char *label;
  valorLexico valor_lexico;
  int table_local_addr;
  bool is_global;
  char *func_label;
  Value valor;
  TipoToken tipo;
} StoredIdentifier;

typedef struct BasicBlock {
    int id;
    char *iloc_code;
    int destiny[2];
    struct BasicBlock *next;
} BasicBlock;

typedef struct LabelMap {
    char* label;
    int block_id;
} LabelMap;

valorLexico atribui_yylval(char *yytext, TipoToken tipo, int num_lines);

Nodo* cria_nodo(valorLexico valor);
Nodo* cria_nodo_v2(valorLexico valor, TipoToken tipo);
void assign_code(Nodo* node, ilocCode *code);
void copy_code_and_free(Nodo* destiny, Nodo* source);
valorLexico cria_call(valorLexico label);
valorLexico cria_valor_lexico(char *label);
valorLexico cria_valor_lexico_v2(char *label, TipoToken tipo);
TipoToken type_infer(Nodo* nodo1, Nodo* nodo2);
void adiciona_filho(Nodo *pai, Nodo *filho);
void print_tree(Nodo *raiz);
void prt_node(void *ptr);
const char* tipoTokenToString(TipoToken tipo);
void exporta(void *tree);
void print_node_code(Nodo* node);

#define FLOW_DEBUG 1

void print_control_flow(void *tree);
BasicBlock* create_new_block(int id);
void add_code_to_block(BasicBlock* block, const char* code);
void gather_basic_blocks(char* code_str, BasicBlock** blocks, int* block_count, LabelMap* label_map, int* label_count);
void analyze_connections(BasicBlock** blocks, int block_count, LabelMap* label_map, int label_count);
void print_blocks(BasicBlock** blocks, int block_count);

int get_line_number();

#endif