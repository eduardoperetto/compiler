
#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "tree.h"
#include "stdbool.h"
#include "error.h"

#define TABLE_SIZE 300

typedef struct Identifier {
  char name[50];
  bool initialized;
  bool isFunction;
  bool isGlobal;
  int declarationLine;
  TipoToken type;
  Value value;
  int local_addr;
  char *func_label;
  struct Identifier* next;
} Identifier;

typedef struct HashTable {
  Identifier* table[TABLE_SIZE];
  bool is_global;
  int curr_offset;
} HashTable;

typedef struct StackNode {
    HashTable* hashTable;
    struct StackNode* next;
} StackNode;

typedef struct HashTableStack {
    StackNode* top;
} HashTableStack;

int get_size(TipoToken tipo);
bool is_inside_main();

HashTable* createTable(bool is_global);
void addIdentifier(HashTableStack** stack, const char* name, TipoToken type, bool isFunction, bool is_global, int line);
Identifier* getIdentifier(HashTable* table, const char* name, bool isFunction, int line);
void freeTable(HashTable* table);
void printTable(HashTable* table);

StackNode* createStackNode(HashTable* hashTable);
void initializeStack(HashTableStack* stack);
void addOnTop(HashTableStack* stack, HashTable* hashTable);
HashTable* getTop(HashTableStack** stack);
HashTable* getLast(HashTableStack** stack);
void dropTop(HashTableStack* stack);
void freeStack(HashTableStack* stack);
void createTableOnTop(HashTableStack** stack);
void updateIdentifier(HashTableStack* stack, char* name, Value newValue, int line);
Identifier* findIdentifier(HashTableStack* stack, char* name, bool isFunction, int line);
void update_func_label(HashTableStack* stack, char* name, char *label);
char* get_func_label(HashTableStack* stack, char* name);
Nodo* getNodeFromId(HashTableStack* stack, char* id, bool isFunction, int line);
Nodo* makeNodeFromIdentifier(Identifier *identifier);
char* checkNatureAndGetLabel(HashTableStack* stack, char* name, bool isFunction, int line);
void update_last_offset(int offset);
int get_last_table_offset();

/* Debug*/
void printErrorPrefix(int line);
void printLine(int line);
void printPrevDeclaration(int line);

#endif