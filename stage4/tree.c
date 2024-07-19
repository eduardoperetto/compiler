
#include "tree.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

valorLexico cria_valor_lexico_v2(char *label, TipoToken tipo) {
  valorLexico vl = {
    .linha = 0,
    .tipo = tipo,
    .valor = 0,
    .label = label
  };
  return vl;
}

valorLexico cria_valor_lexico(char *label) {
  valorLexico vl = {
    .linha = 0,
    .tipo = NONE,
    .valor = 0,
    .label = label
  };
  return vl;
}

valorLexico cria_call(valorLexico func) {
    int length = strlen("call ") + strlen(func.label) + 1;

    valorLexico vl = {
        .linha = 0,
        .tipo = NONE,
        .valor = 0,
        .label = (char *)malloc(length * sizeof(char))
    };

    if (vl.label != NULL) {
        sprintf(vl.label, "call %s", func.label);
    }

    return vl;
}

Nodo *cria_nodo_v2(valorLexico valor, TipoToken tipo) {
  Nodo *novoNodo = malloc(sizeof(Nodo));
  novoNodo->valor_lexico = valor;
  novoNodo->filhos = NULL;
  novoNodo->num_filhos = 0;
  novoNodo->tipo = tipo;
  prt_node(novoNodo);
  return novoNodo;
}

Nodo *cria_nodo(valorLexico valor) {
  Nodo *novoNodo = malloc(sizeof(Nodo));
  novoNodo->valor_lexico = valor;
  novoNodo->filhos = NULL;
  novoNodo->num_filhos = 0;
  novoNodo->tipo = NONE;
  prt_node(novoNodo);
  return novoNodo;
}

void adiciona_filho(Nodo *pai, Nodo *filho) {
  if (pai == NULL || filho == NULL) {
    return;
  }

  int qtd = pai->num_filhos + 1;
  Nodo **new_filhos = realloc(pai->filhos, qtd * sizeof(Nodo *));
  if (new_filhos == NULL) {
    return;
  }

  pai->filhos = new_filhos;
  pai->filhos[pai->num_filhos] = filho;
  pai->num_filhos = qtd;
}

valorLexico atribui_yylval(char *yytext, TipoToken tipo, int num_lines) {
  valorLexico valor_lexico;
  valor_lexico.linha = num_lines;
  valor_lexico.tipo = tipo;
  valor_lexico.label = strdup(yytext);

  switch (tipo) {
    case BOOL:
      bool value = strcmp(yytext, "true") == 0;
      valor_lexico.valor.b_val = value ? true : false;
      break;
    case INT:
      valor_lexico.valor.i_val = atoi(yytext);
      break;
    case FLOAT:
      valor_lexico.valor.f_val = atof(yytext);
      break;
  }

  return valor_lexico;
}

TipoToken type_infer(Nodo* nodo1, Nodo* nodo2) {
  // (int, int) → int; 
  if (nodo1->tipo == INT && nodo2->tipo == INT) {
    return INT;
  }
  // (float, float) → float; 
  if (nodo1->tipo == FLOAT && nodo2->tipo == FLOAT) {
    return FLOAT;
  }
  // (bool, bool) → bool; 
  if (nodo1->tipo == BOOL && nodo2->tipo == BOOL) {
    return BOOL;
  }
  // (float, int) → float;
  if ((nodo1->tipo == FLOAT && nodo2->tipo == INT) || (nodo1->tipo == INT && nodo2->tipo == FLOAT)) {
    return FLOAT;
  }
  // (bool, int) → int; 
  if ((nodo1->tipo == BOOL && nodo2->tipo == INT) || (nodo1->tipo == INT && nodo2->tipo == BOOL)) {
    return INT;
  }
  // (bool, float) → float
  if ((nodo1->tipo == BOOL && nodo2->tipo == FLOAT) || (nodo1->tipo == FLOAT && nodo2->tipo == BOOL)) {
    return FLOAT;
  }
  return NONE;
}

void print_node_label(Nodo *nodo) {
    if (nodo && nodo->valor_lexico.label) {
        printf("%p [label=\"%s\"];\n", (void *)nodo, nodo->valor_lexico.label);
    } else {
        printf("%p [label=\"\"];\n", (void *)nodo);
    }
}

void print_tree_labels(Nodo *raiz) {
    if (!raiz) {
        return;
    }

    print_node_label(raiz);

    for (int i = 0; i < raiz->num_filhos; i++) {
        if (raiz->filhos[i]) {
            print_tree_labels(raiz->filhos[i]);
        }
    }
}

void print_node_addresses(Nodo *raiz) {
    if (!raiz) {
        return;
    }

    for (int i = 0; i < raiz->num_filhos; i++) {
        if (raiz->filhos[i]) {
            printf("%p, %p\n", (void *)raiz, (void *)raiz->filhos[i]);
            print_node_addresses(raiz->filhos[i]);
        }
    }
}

void exporta(void *arvore) {
  #if PRINT_TREE_ADDR
  Nodo *nodo_arvore;
  nodo_arvore = (Nodo *)arvore;
  print_node_addresses(nodo_arvore);
  print_tree_labels(nodo_arvore);
  #endif
  return;
}


const char* tipoTokenToString(TipoToken tipo) {
    switch (tipo) {
        case INT: return "INT";
        case FLOAT: return "FLOAT";
        case BOOL: return "BOOL";
        case NONE: return "NONE";
        default: return "UNKNOWN";
    }
}

void prt_node(void *ptr) {
    #if DEBUG_NODES
    if (ptr == NULL) {
        printf("Nodo is NULL\n");
        return;
    }
    Nodo* nodo = (Nodo *) ptr;
    
    printf("Nodo {\n");
    printf("  valor_lexico {\n");
    printf("    linha: %d\n", nodo->valor_lexico.linha);
    printf("    tipo: %s\n", tipoTokenToString(nodo->valor_lexico.tipo));

    if (nodo->valor_lexico.tipo == FLOAT) {
        float value = nodo->valor_lexico.valor.f_val;
        printf("    valor: %f\n", value);
    } else if (nodo->valor_lexico.tipo == INT) {
        int value = nodo->valor_lexico.valor.i_val;
        printf("    valor: %d\n", value);
    } else if (nodo->valor_lexico.tipo == BOOL) {
        int value = nodo->valor_lexico.valor.b_val;
        printf("    valor: %s\n", value ? "True" : "False");
    } else {
        printf("    valor: None\n");
    }
    if (nodo->valor_lexico.label != NULL) {
        printf("    label: %s\n", nodo->valor_lexico.label);
    } else {
        printf("    label: NULL\n");
    }
	  printf("  }\n");
    printf("  tipo: %s\n", tipoTokenToString(nodo->tipo));
    printf("}\n");
    #endif
}
