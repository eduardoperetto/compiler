
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
  newIdentifier->initialized = false;
  newIdentifier->next = NULL;
  return newIdentifier;
}

void addIdentifier(HashTable* table, const char* name, TipoToken type, bool isFunction) {
  if (table == NULL) {
    table = createTable();
  }
  unsigned int index = hash(name);
  Identifier* newIdentifier = createIdentifier(name, type, isFunction);
  if (table->table[index] == NULL) {
    table->table[index] = newIdentifier;
  } else {
    Identifier* current = table->table[index];
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = newIdentifier;
  }
}

void updateIdentifier(HashTable* table, const char* name, Value newValue) {
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
}

Identifier* getIdentifier(HashTable* table, const char* name) {
  unsigned int index = hash(name);
  Identifier* current = table->table[index];
  while (current != NULL) {
    if (strcmp(current->name, name) == 0) {
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
  for (int i = 0; i < TABLE_SIZE; i++) {
    Identifier* current = table->table[i];
    while (current != NULL) {
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