
#include "hash_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HashTableStack* tableStack;
HashTable* globalTable;

static int last_table_offset = 0;
static bool main_declared = false;
static bool finished_main = false;

bool is_inside_main() {
  return main_declared && !finished_main;
}

/* Hash table */

int get_size(TipoToken tipo) {
    switch (tipo) {
        case INT: return 4;
        case FLOAT: return 8;
        case BOOL: return 1;
        case NONE: return 0;
    }
}

// Gera um hash a partir do identificador ASCII
unsigned int hash(const char* key) {
  unsigned int hash = 0;
  while (*key) {
    hash = (hash << 5) + *key++;
  }
  return hash % TABLE_SIZE;
}

HashTable* createTable() {
  HashTable* newTable = (HashTable*)malloc(sizeof(HashTable));
  for (int i = 0; i < TABLE_SIZE; i++) {
    newTable->table[i] = NULL;
  }
  return newTable;
}

Identifier* createIdentifier(const char* name, TipoToken type, bool isFunction, int line, int local_addr) {
  Identifier* newIdentifier = (Identifier*)malloc(sizeof(Identifier));
  strcpy(newIdentifier->name, name);
  newIdentifier->type = type;
  newIdentifier->isFunction = isFunction;
  newIdentifier->initialized = isFunction;
  newIdentifier->declarationLine = line;
  newIdentifier->local_addr = local_addr;
  newIdentifier->next = NULL;
  return newIdentifier;
}

void addIdentifier(HashTable* table, const char* name, TipoToken type, bool isFunction, int line) {
  if (table == NULL) {
    table = createTable();
  }
  unsigned int index = hash(name);
  int local_addr = table->curr_offset;
  table->curr_offset += get_size(type);
  Identifier* newIdentifier = createIdentifier(name, type, isFunction, line, local_addr);
  if (table->table[index] == NULL) {
    table->table[index] = newIdentifier;
  } else {
    printErrorPrefix(line);
    printf("Identificador '%s' já foi declarado.\n", name);
    printPrevDeclaration((table->table[index])->declarationLine);
    exit(ERR_DECLARED);
  }
  if (strcmp(name, "main") == 0) {
    main_declared = true;
  }
}

Identifier* getIdentifier(HashTable* table, const char* name, bool isFunction, int line) {
  unsigned int index = hash(name);
  Identifier* current = table->table[index];
  while (current != NULL) {
    if (strcmp(current->name, name) == 0) {
      if ((current->isFunction) && !isFunction) {
        printErrorPrefix(line);
        printf("Identificador '%s' foi declarado como função, e não pode ser usado neste contexto.\n", name);
        printPrevDeclaration(current->declarationLine);
        exit(ERR_FUNCTION);
      }
      if (!(current->isFunction) && isFunction) {
        printErrorPrefix(line);
        printf("Identificador '%s' foi declarado como variável, e não pode ser usado neste contexto.\n", name);
        printPrevDeclaration(current->declarationLine);
        exit(ERR_VARIABLE);
      }
      return current;
    }
    current = current->next;
  }
  return NULL;
}

void freeTable(HashTable* table) {
  for (int i = 0; i < TABLE_SIZE; i++) {
    Identifier* current = table->table[i];
    while (current != NULL) {
      Identifier* toDelete = current;
      current = current->next;
      free(toDelete);
    }
  }
  free(table);
}

void printTable(HashTable* table) {
  bool allIsNull = true;
  for (int i = 0; i < TABLE_SIZE; i++) {
    Identifier* current = table->table[i];
    while (current != NULL) {
      allIsNull = false;
      const char* tipoStr = tipoTokenToString(current->type);
      printf("Name: %s, Type: %s, ", current->name, tipoStr);
      printf("IsFunction: %s, ", current->isFunction ? "true" : "false");
      printf("Initialized: %s, ", current->initialized ? "true" : "false");
      printf("LocalAddr: %d, ", current->local_addr);
      printf("Function label: %s, ", current->isFunction ? current->func_label : "Null");
      switch (current->type) {
        case BOOL:
          printf("Value: %d\n", current->value.b_val);
          break;
        case FLOAT:
          printf("Value: %.2f\n", current->value.f_val);
          break;
        case INT:
          printf("Value: %d\n", current->value.i_val);
          break;
      }
      current = current->next;
    }
  }
  printf("Table current offset: %d\n", table->curr_offset);
  #if DEBUG_PARSER
  if (allIsNull) {
    printf("Table has no symbols\n");
  }
  #endif
}

/* Stack */

StackNode* createStackNode(HashTable* hashTable) {
  StackNode* stackNode = (StackNode*)malloc(sizeof(StackNode));
  stackNode->hashTable = hashTable;
  stackNode->next = NULL;
  return stackNode;
}

void initializeStack(HashTableStack* stack) {
  stack->top = NULL;
}

void addOnTop(HashTableStack* stack, HashTable* hashTable) {
  StackNode* stackNode = createStackNode(hashTable);
  stackNode->next = stack->top;
  stack->top = stackNode;
}

void createTableOnTop(HashTableStack** stack) {
  if (*stack == NULL) {
    *stack = (HashTableStack*)malloc(sizeof(HashTableStack));
    initializeStack(*stack);
    (*stack)->top = createStackNode(createTable());
  }
  HashTable* newTable = createTable();
  addOnTop(*stack, newTable);
}

HashTable* getTop(HashTableStack** stack) {
  if (*stack == NULL) {
    *stack = (HashTableStack*)malloc(sizeof(HashTableStack));
    initializeStack(*stack);
    (*stack)->top = createStackNode(createTable());
  }
  if ((*stack)->top == NULL) {
    return NULL;
  }
  return (*stack)->top->hashTable;
}

HashTable* getLast(HashTableStack** stack) {
  if (*stack == NULL) {
    *stack = (HashTableStack*)malloc(sizeof(HashTableStack));
    initializeStack(*stack);
    (*stack)->top = createStackNode(createTable());
  }
  StackNode* current = (*stack)->top;
  while (current->next != NULL) {
    current = current->next;
  }
  return current->hashTable;
}

void dropTop(HashTableStack* stack) {
  if (stack->top == NULL) {
    return;
  }
  StackNode* temp = stack->top;
  stack->top = stack->top->next;
  free(temp->hashTable);
  free(temp);
}

void freeStack(HashTableStack* stack) {
  while (stack->top != NULL) {
    dropTop(stack);
  }
}

void updateIdentifier(HashTableStack* stack, char* name, Value newValue, int line) {
  unsigned int index = hash(name);
  Identifier* identifier = findIdentifier(stack, name, false, line);
  if (identifier == NULL) {
    printErrorPrefix(line);
    printf("Variável '%s' não foi declarada.\n", name);
    exit(ERR_UNDECLARED);
  }
  identifier->value = newValue;
}

void update_func_label(HashTableStack* stack, char* name, char *label) {
  unsigned int index = hash(name);
  Identifier* identifier = findIdentifier(stack, name, true, -1);
  if (identifier == NULL) {
    printErrorPrefix(-1);
    printf("Erro interno (função %s não encontrada ao adicionar label).\n", name);
    exit(ERR_UNDECLARED);
  }
  identifier->func_label = label;

  if (strcmp(name, "main") == 0) {
    finished_main = true;
  }
}

char* get_func_label(HashTableStack* stack, char* name) {
  unsigned int index = hash(name);
  Identifier* identifier = findIdentifier(stack, name, true, get_line_number());
  if (identifier == NULL) {
    printErrorPrefix(get_line_number());
    printf("Erro interno (função %s não encontrada ao recuperar label).\n", name);
    exit(ERR_UNDECLARED);
  }
  return identifier->func_label;
}

Identifier* findIdentifier(HashTableStack* stack, char* name, bool isFunction, int line) {
  if (stack == NULL || stack->top == NULL) {
    printErrorPrefix(line);
    printf("Variável '%s' não foi declarada.\n", name);
    exit(ERR_UNDECLARED);
  }

  StackNode* currentStackNode = stack->top;
  Identifier* identifier = NULL;

  while (currentStackNode != NULL) {
    identifier = getIdentifier(currentStackNode->hashTable, name, isFunction, line);
    if (identifier != NULL) {
      return identifier;
    }
    currentStackNode = currentStackNode->next;
  }
  return NULL;
}

Nodo* getNodeFromId(HashTableStack* stack, char* name, bool isFunction, int line) {
  Identifier* identifier = findIdentifier(stack, name, isFunction, line);

  if (identifier == NULL) {
    printErrorPrefix(line);
    printf("Variável '%s' não foi declarada.\n", name);
    exit(ERR_UNDECLARED);
  }

  Nodo* newNode = (Nodo*)malloc(sizeof(Nodo));
  newNode->valor_lexico.linha = -1;
  newNode->valor_lexico.tipo = identifier->type;
  newNode->valor_lexico.label = strdup(identifier->name);
  newNode->num_filhos = 0;
  newNode->table_local_addr = identifier->local_addr;
  newNode->filhos = NULL;
  newNode->tipo = identifier->type;

  switch (identifier->type) {
    case BOOL:
      newNode->valor_lexico.valor.b_val = identifier->value.b_val;
      break;
    case FLOAT:
      newNode->valor_lexico.valor.f_val = identifier->value.f_val;
      break;
    case INT:
      newNode->valor_lexico.valor.i_val = identifier->value.i_val;
      break;
    default:
      break;
  }

  return newNode;
}

char* checkNatureAndGetLabel(HashTableStack* stack, char* name, bool isFunction, int line) {
  StackNode* currentStackNode = stack->top;
  Identifier* identifier = NULL;

  while (currentStackNode != NULL) {
    identifier = getIdentifier(currentStackNode->hashTable, name, isFunction, line);
    if (identifier != NULL) {
      break;
    }
    currentStackNode = currentStackNode->next;
  }
  if (identifier == NULL) {
    printErrorPrefix(line);
    printf("Identificador '%s' não foi declarado.\n", name);
    exit(ERR_UNDECLARED);
  }
  return identifier->func_label;
}

void update_last_offset(int offset) {
  last_table_offset = offset;
}

int get_last_table_offset() {
  int offset = last_table_offset;
  return offset;
}

/* Debug functions */
void printErrorPrefix(int line) {
    printLine(line);
    printf("\033[1;31m[Error] \033[0m");
}

void printPrevDeclaration(int line) {
    printf("\033[0;36m(Previamente declarado na linha %d)\033[0m\n", line);
}

void printLine(int line) {
    printf("\033[0;36m(Linha %d): \033[0m", line);
}