
#include "tree.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

valorLexico cria_valor_lexico(char *label) {
  valorLexico vl = {
    .linha = 0,
    .tipo = NONE,
    .valorToken = NULL,
    .label = label
  };
  return vl;
}

valorLexico cria_call(valorLexico func) {
    int length = strlen("call ") + strlen(func.label) + 1;

    valorLexico vl = {
        .linha = 0,
        .tipo = NONE,
        .valorToken = NULL,
        .label = (char *)malloc(length * sizeof(char))
    };

    if (vl.label != NULL) {
        // Use sprintf to concatenate "call " and label directly into vl.label
        sprintf(vl.label, "call %s", func.label);
    }

    return vl;
}

Nodo *cria_nodo(valorLexico valor) {
  Nodo *novoNodo = malloc(sizeof(Nodo));
  novoNodo->valor_lexico = valor;
  novoNodo->filhos = NULL;
  novoNodo->num_filhos = 0;
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
    case LIT_BOOL:
    case LIT_FLOAT:
    case LIT_INTEIRO:
    case LIT_STRING:
      valor_lexico.valorToken = strdup(yytext + 1);
      break;
    case LIT_CHAR:
      char *new_chr = malloc(sizeof(char) + 1);
      new_chr[0] = (yytext + 1)[1];
      new_chr[1] = '\0';
      valor_lexico.valorToken = new_chr;
      break;
  }

  return valor_lexico;
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

void exporta(void *tree) {
  Nodo *nodo_tree;
  nodo_tree = (Nodo *)tree;
  print_node_addresses(nodo_tree);
  print_tree_labels(nodo_tree);
  return;
}
