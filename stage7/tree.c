
#include "tree.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

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
  novoNodo->table_local_addr = -1;
  novoNodo->iloc_code = NULL;
  novoNodo->temp_reg = NULL;
  novoNodo->temp_reg_false = NULL;
  prt_node(novoNodo);
  return novoNodo;
}

Nodo *cria_nodo(valorLexico valor) {
  Nodo *novoNodo = malloc(sizeof(Nodo));
  novoNodo->valor_lexico = valor;
  novoNodo->filhos = NULL;
  novoNodo->num_filhos = 0;
  novoNodo->table_local_addr = -1;
  novoNodo->tipo = valor.tipo;
  novoNodo->temp_reg_false = NULL;
  novoNodo->temp_reg = NULL;
  prt_node(novoNodo);
  return novoNodo;
}

void assign_code(Nodo* node, ilocCode *code) {
  if (node == NULL || code == NULL) return;
  node->iloc_code = code;
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
      valor_lexico.valor.i_val = value ? 1 : 0;
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

void exporta(void *tree) {
  #if PRINT_TREE_ADDR
  Nodo *nodo_tree;
  nodo_tree = (Nodo *)tree;
  print_node_addresses(nodo_tree);
  print_tree_labels(nodo_tree);
  #endif
  return;
}
void print_control_flow(void *tree) {
    Nodo* root = (Nodo *)tree;
    char* code_str = get_code_as_string(root->iloc_code);

    BasicBlock* blocks[100]; // Suporte para até 100 blocos
    LabelMap label_map[100]; // Suporte para até 100 labels
    int block_count = 0;
    int label_count = 0;

    // Identificar e coletar os blocos básicos e labels
    if (FLOW_DEBUG) printf("Iniciando a coleta dos blocos básicos e labels...\n");
    gather_basic_blocks(code_str, blocks, &block_count, label_map, &label_count);

    // Analisar as conexões entre os blocos
    if (FLOW_DEBUG) printf("Iniciando a análise das conexões entre os blocos...\n");
    analyze_connections(blocks, block_count, label_map, label_count);

    // Imprimir os blocos básicos e as conexões
    if (FLOW_DEBUG) printf("Imprimindo os blocos e suas conexões...\n");
    print_blocks(blocks, block_count);

    // Liberar memória alocada
    free(code_str);
    for (int i = 0; i < block_count; i++) {
        free(blocks[i]->iloc_code);
        free(blocks[i]);
    }
    for (int i = 0; i < label_count; i++) {
        free(label_map[i].label);
    }
}

BasicBlock* create_new_block(int id) {
    if (FLOW_DEBUG) printf("Criando um novo bloco com id %d\n", id);
    BasicBlock* block = (BasicBlock*)malloc(sizeof(BasicBlock));
    if (!block) {
        perror("Failed to allocate memory for BasicBlock");
        exit(EXIT_FAILURE);
    }
    block->id = id;
    block->iloc_code = NULL;
    block->destiny[0] = -1;
    block->destiny[1] = -1;
    return block;
}

void add_code_to_block(BasicBlock* block, const char* code) {
    if (FLOW_DEBUG) printf("Adicionando código ao bloco %d: %s\n", block->id, code);
    if (!block->iloc_code) {
        block->iloc_code = strdup(code);
        if (!block->iloc_code) {
            perror("Failed to allocate memory for iloc_code");
            exit(EXIT_FAILURE);
        }
    } else {
        size_t new_size = strlen(block->iloc_code) + strlen(code) + 2;
        block->iloc_code = realloc(block->iloc_code, new_size);
        if (!block->iloc_code) {
            perror("Failed to reallocate memory for iloc_code");
            exit(EXIT_FAILURE);
        }
        strcat(block->iloc_code, "\n");
        strcat(block->iloc_code, code);
    }
}

void gather_basic_blocks(char* code_str, BasicBlock** blocks, int* block_count, LabelMap* label_map, int* label_count) {
    char* line = strtok(code_str, "\n");
    BasicBlock* current_block = create_new_block((*block_count)++);
    blocks[current_block->id] = current_block;

    while (line) {
        // Verificar se a linha é um label
        regex_t regex_label;
        regcomp(&regex_label, "^[L][A-Za-z0-9_]*:", REG_EXTENDED);
        if (strstr(line, "jumpI") != NULL || strstr(line, "jump ") != NULL || strstr(line, "cbr") != NULL) {
            if (FLOW_DEBUG) printf("Encontrado branch: %s\n", line);
            regfree(&regex_label);

            add_code_to_block(current_block, line);
            line = strtok(NULL, "\n");
            if (current_block->iloc_code) {
                current_block = create_new_block((*block_count)++);
                blocks[current_block->id] = current_block;
            }
            continue;
        }
        else if (regexec(&regex_label, line, 0, NULL, 0) == 0) {
            if (FLOW_DEBUG) printf("Encontrado label: %s\n", line);
            // Se o bloco atual já tiver código, iniciar um novo bloco
            if (current_block->iloc_code) {
                current_block = create_new_block((*block_count)++);
                blocks[current_block->id] = current_block;
            }

            // Extrair o nome do label
            char* colon_pos = strchr(line, ':');
            size_t label_len = colon_pos - line;
            char* label_name = strndup(line, label_len);

            // Mapear o label para o bloco atual
            label_map[*label_count].label = label_name;
            label_map[*label_count].block_id = current_block->id;
            if (FLOW_DEBUG) printf("Label %s mapeado para o bloco %d\n", label_name, current_block->id);
            (*label_count)++;
        }
        regfree(&regex_label);

        add_code_to_block(current_block, line);
        line = strtok(NULL, "\n");
    }
}

int find_block_id_by_label(LabelMap* label_map, int label_count, const char* label) {
    if (FLOW_DEBUG) printf("Procurando o bloco para o label %s...\n", label);
    for (int i = 0; i < label_count; i++) {
        if (strcmp(label_map[i].label, label) == 0) {
            if (FLOW_DEBUG) printf("Label %s encontrado no bloco %d\n", label, label_map[i].block_id);
            return label_map[i].block_id;
        }
    }
    if (FLOW_DEBUG) printf("Label %s não encontrado!\n", label);
    return -1; // Label não encontrado
}

void analyze_connections(BasicBlock** blocks, int block_count, LabelMap* label_map, int label_count) {
    for (int i = 0; i < block_count; i++) {
        char* code = blocks[i]->iloc_code;
        char* last_line = strrchr(code, '\n');
        if (last_line) {
            last_line++; // Pular o caractere '\n'
        } else {
            last_line = code; // Apenas uma linha de código no bloco
        }

        if (FLOW_DEBUG) printf("Analisando a última linha do bloco %d: %s\n", blocks[i]->id, last_line);

        // Verificar se a última linha é um jumpI ou jump
        if (strstr(last_line, "jumpI") != NULL || strstr(last_line, "jump ") != NULL) {
            if (FLOW_DEBUG) printf("Instrução de jumpI/jump encontrada no bloco %d\n", blocks[i]->id);
            // Extrair o label de destino
            regex_t regex_jump;
            regcomp(&regex_jump, "->\\s*([A-Za-z_][A-Za-z0-9_]*)", REG_EXTENDED);
            regmatch_t matches[2];
            if (regexec(&regex_jump, last_line, 2, matches, 0) == 0) {
                char label_name[64];
                int len = matches[1].rm_eo - matches[1].rm_so;
                strncpy(label_name, last_line + matches[1].rm_so, len);
                label_name[len] = '\0';

                int dest_block = find_block_id_by_label(label_map, label_count, label_name);
                if (dest_block != -1) {
                    blocks[i]->destiny[0] = dest_block;
                    if (FLOW_DEBUG) printf("Bloco %d -> Bloco %d\n", blocks[i]->id, dest_block);
                }
            }
            regfree(&regex_jump);
        }
        // Verificar se a última linha é um cbr
        else if (strstr(last_line, "cbr") != NULL) {
            if (FLOW_DEBUG) printf("Instrução de cbr encontrada no bloco %d\n", blocks[i]->id);
            // Extrair os dois labels de destino
            regex_t regex_cbr;
            regcomp(&regex_cbr, "->\\s*([A-Za-z_][A-Za-z0-9_]*),\\s*([A-Za-z_][A-Za-z0-9_]*)", REG_EXTENDED);
            regmatch_t matches[3];
            if (regexec(&regex_cbr, last_line, 3, matches, 0) == 0) {
                char label_name1[64];
                char label_name2[64];

                int len1 = matches[1].rm_eo - matches[1].rm_so;
                int len2 = matches[2].rm_eo - matches[2].rm_so;

                strncpy(label_name1, last_line + matches[1].rm_so, len1);
                strncpy(label_name2, last_line + matches[2].rm_so, len2);

                label_name1[len1] = '\0';
                label_name2[len2] = '\0';

                int dest_block1 = find_block_id_by_label(label_map, label_count, label_name1);
                int dest_block2 = find_block_id_by_label(label_map, label_count, label_name2);

                if (dest_block1 != -1) {
                    blocks[i]->destiny[0] = dest_block1;
                    if (FLOW_DEBUG) printf("Bloco %d -> Bloco %d\n", blocks[i]->id, dest_block1);
                }
                if (dest_block2 != -1) {
                    blocks[i]->destiny[1] = dest_block2;
                    if (FLOW_DEBUG) printf("Bloco %d -> Bloco %d\n", blocks[i]->id, dest_block2);
                }
            }
            regfree(&regex_cbr);
        }
        // Conexão sequencial padrão
        else if (i < block_count - 1) {
            blocks[i]->destiny[0] = blocks[i + 1]->id;
            if (FLOW_DEBUG) printf("Conexão sequencial: Bloco %d -> Bloco %d\n", blocks[i]->id, blocks[i + 1]->id);
        }
    }
}

void print_blocks(BasicBlock** blocks, int block_count) {
    if (FLOW_DEBUG) {
      for (int i = 0; i < block_count; i++) {
          printf("B%d:\n%s\n", blocks[i]->id, blocks[i]->iloc_code);
      }
    }

    printf("digraph {\n");
    for (int i = 0; i < block_count; i++) {
        if (blocks[i]->destiny[0] != -1) {
            printf("\tB%d -> B%d\n", blocks[i]->id, blocks[i]->destiny[0]);
        }
        if (blocks[i]->destiny[1] != -1) {
            printf("\tB%d -> B%d\n", blocks[i]->id, blocks[i]->destiny[1]);
        }
    }
    printf("}\n");
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

void print_node_code(Nodo* node) {
  print_code(node->iloc_code);
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
    printf("  temp_reg: %s\n", nodo->temp_reg);
    print_node_code(nodo);
    printf("}\n");
    #endif
}
