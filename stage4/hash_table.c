
#include "hash_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HashTableStack* tableStack;
HashTable* globalTable;

/* Hash table */

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

Identifier* createIdentifier(const char* name, TipoToken type, bool isFunction) {
  Identifier* newIdentifier = (Identifier*)malloc(sizeof(Identifier));
  strcpy(newIdentifier->name, name);
  newIdentifier->type = type;
  newIdentifier->isFunction = isFunction;
  newIdentifier->initialized = isFunction;
  newIdentifier->next = NULL;
  return newIdentifier;
}

void addIdentifier(HashTable* table, const char* name, TipoToken type, bool isFunction, int line) {
  if (table == NULL) {
    table = createTable();
  }
  unsigned int index = hash(name);
  Identifier* newIdentifier = createIdentifier(name, type, isFunction);
  if (table->table[index] == NULL) {
    table->table[index] = newIdentifier;
  } else {
    printf("[ERRO] Identificador '%s' já foi declarado (linha %d).\n", name, line);
    exit(ERR_DECLARED);
  }
}

void updateIdentifier(HashTable* table, const char* name, Value newValue, int line) {
  unsigned int index = hash(name);
  Identifier* current = table->table[index];
  while (current != NULL) {
    if (strcmp(current->name, name) == 0) {
      current->value = newValue;
      current->initialized = true;
      return;
    }
    current = current->next;
  }
  printf("[ERRO] Variável '%s' não foi declarada (linha %d).\n", name, line);
  exit(ERR_UNDECLARED);
}

Identifier* getIdentifier(HashTable* table, const char* name, bool isFunction, int line) {
  unsigned int index = hash(name);
  Identifier* current = table->table[index];
  while (current != NULL) {
    if (strcmp(current->name, name) == 0) {
      if ((current->isFunction) && !isFunction) {
        printf("[ERRO] Identificador '%s' foi declarado como função, e não pode ser usado neste contexto (linha %d).\n", name, line);
        exit(ERR_FUNCTION);
      }
      if (!(current->isFunction) && isFunction) {
        printf("[ERRO] Identificador '%s' foi declarado como variável, e não pode ser usado neste contexto (linha %d).\n", name, line);
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
  #ifdef DEBUG_PARSER
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

void createTableOnTop(HashTableStack* stack) {
  HashTable* newTable = createTable();
  addOnTop(stack, newTable);
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

HashTable* getLast(HashTableStack* stack) {
  if (stack->top == NULL) {
    return NULL;
  }
  StackNode* current = stack->top;
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

Nodo* getNodeFromId(HashTableStack* stack, char* name, bool isFunction, int line) {
  if (stack == NULL || stack->top == NULL) {
    printf("[ERRO] Variável '%s' não foi declarada (linha %d).\n", name, line);
    exit(ERR_UNDECLARED);
  }

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
    printf("[ERRO] Variável '%s' não foi declarada (linha %d).\n", name, line);
    exit(ERR_UNDECLARED);
  }


  Nodo* newNode = (Nodo*)malloc(sizeof(Nodo));
  newNode->valor_lexico.linha = -1;
  newNode->valor_lexico.tipo = identifier->type;
  newNode->valor_lexico.label = strdup(identifier->name);
  newNode->num_filhos = 0;
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

void checkNature(HashTableStack* stack, char* name, bool isFunction, int line) {
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
    printf("[ERRO] Identificador '%s' não foi declarado (linha %d).\n", name, line);
    exit(ERR_UNDECLARED);
  }
}
