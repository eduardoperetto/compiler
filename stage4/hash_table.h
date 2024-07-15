
#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "tree.h"
#include "stdbool.h"

#define TABLE_SIZE 300

typedef struct Identifier {
  char name[50];
  bool initialized;
  bool isFunction;
  TipoToken type;
  Value value;
  struct Identifier* next;
} Identifier;

typedef struct HashTable {
  Identifier* table[TABLE_SIZE];
} HashTable;

typedef struct StackNode {
    HashTable* hashTable;
    struct StackNode* next;
} StackNode;

typedef struct {
    StackNode* top;
} HashTableStack;

HashTable* createTable();
void addIdentifier(HashTable* table, const char* name, TipoToken type, bool isFunction);
void updateIdentifier(HashTable* table, const char* name, Value newValue);
Identifier* getIdentifier(HashTable* table, const char* name);
void freeTable(HashTable* table);
void printTable(HashTable* table);

StackNode* createStackNode(HashTable* hashTable);
void initializeStack(HashTableStack* stack);
void addOnTop(HashTableStack* stack, HashTable* hashTable);
HashTable* getTop(HashTableStack** stack);
HashTable* getLast(HashTableStack* stack);
void dropTop(HashTableStack* stack);
void freeStack(HashTableStack* stack);

#endif