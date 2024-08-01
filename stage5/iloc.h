#ifndef ILOC_H
#define ILOC_H
#include <stdlib.h>
#include <stdbool.h>

/* Forward declarations */
typedef struct Nodo Nodo;
typedef struct HashTableStack HashTableStack;
typedef struct HashTable HashTable;

#define DEBUG_ILOC 1

typedef enum ILOCOperator {
    NOP, // Não faz nada
    HALT,

    /* Operações */
    ADD, // R3 = R1 + R2
    SUB, // R3 = R1 - R2
    MULT, // R3 = R1 * R2
    DIV, // R3 = R1 / R2
    ADDI, // R3 = R1 + C2
    SUBI, // R3 = R1 - C2
    RSUBI, // R3 = C2 - R1
    MULTI, // R3 = R1 * C2
    DIVI, // R3 = R1 / C2
    RDIVI, // R3 = C2 / R1
    LSHIFT, // R3 = R1 << R2
    LSHIFTI, // R3 = R1 << C2
    RSHIFT, // R3 = R1 >> R2
    RSHIFTI, // R3 = R1 >> C2
    AND, // R3 = R1 && R2
    ANDI, // R3 = R1 && C2
    OR, // R3 = R1 || R2
    ORI, // R3 = R1 || C2
    XOR, // R3 = R1 XOR R2
    XORI, // R3 = R1 XOR C2
    LOADI, // R2 = C1
    LOAD, // R2 = MEMORIA(R1)
    LOADAI, // R3 = MEMORIA(R1 + C2)
    LOADA0, // R3 = MEMORIA(R1 + R2)
    CLOAD, // Caractere Load
    CLOADAI, // Caractere LoadAI
    CLOADA0, // Caractere LoadA0
    STORE, // Memoria(r2) = R1
    STOREAI, // Memoria(r2 + c3) = R1
    STOREAO, // Memoria(r2 + r3) = R1
    CSTORE, // Caractere Store
    CSTOREAI, // Caractere StoreAI
    CSTOREAO, // Caractere StoreAO
    I2I, // R2 = R1 Para Inteiros
    C2C, // R2 = R1 Para Caracteres
    C2I, // Converte Um Caractere Para Um Inteiro
    I2C, // Converte Um Inteiro Para Caractere

    /* Fluxo de controle */
    JUMPI, // l1 ; PC = endereço(l1)
    JUMP, // r1 ; PC = r1
    CBR, // r1, l2, l3 ; PC = endereço(l2) se r1 = true, senão PC = endereço(l3)
    CMP_LT, // r1, r2, r3 ; r3 = true se r1 < r2, senão r3 = false
    CMP_LE, // r1, r2, r3 ; r3 = true se r1 \leq r2, senão r3 = false
    CMP_EQ, // r1, r2, r3 ; r3 = true se r1 = r2, senão r3 = false
    CMP_GE, // r1, r2, r3 ; r3 = true se r1 \geq r2, senão r3 = false
    CMP_GT, // r1, r2, r3 ; r3 = true se r1 > r2, senão r3 = false
    CMP_NE, // r1, r2, r3 ; r3 = true se r1 \ne r2, senão r3 = false
} ilocOp;

typedef struct ILOCArgument
{
    char *label;
    char *temp_reg;
    int imediate_value;
} ilocArg;

typedef struct ILOCCode
{
    ilocOp operation;
    ilocArg *arg1;
    ilocArg *arg2;
    ilocArg *arg3;
    struct ILOCCode *next;
} ilocCode;

void mock_code(Nodo *node);
char* gen_label();
char* gen_temp();
ilocCode* gen_code(ilocOp operation, ilocArg *arg1, ilocArg *arg2, ilocArg *arg3);
ilocArg* build_arg_im_value(int im_value);
ilocArg* build_arg_label(char* label);
ilocArg* build_arg_temp(char* temp_reg);
ilocCode* findLastNode(ilocCode* list);
ilocCode* merge_code(ilocCode* existingList, ilocCode* newList);
void append_node_codes(Nodo* destiny, Nodo* child);
ilocCode* gen_label_with_nop();
ilocArg* gen_temp_as_arg();
ilocArg* rfp_arg();
ilocArg* rbss_arg();
void gen_func_declaration(Nodo *header, Nodo *body, HashTableStack *table_stack);
void gen_load_var(Nodo *var, bool is_global);
void gen_load_literal(Nodo *val_node);
void gen_assignment(Nodo *assign_node, Nodo* arg3, Nodo* expr);
void gen_return(Nodo *return_node, Nodo *expr_node);
void gen_bin_expr(Nodo *root, Nodo *arg1, Nodo *arg2);
void gen_invert_signal(Nodo *root, Nodo *arg);
void gen_logic_invert(Nodo *root, Nodo *arg);
void gen_while(Nodo *root_while);
void gen_if(Nodo *root_if, Nodo* expr, Nodo *true_block, Nodo *else_block);
void gen_call_func(Nodo *call_node, Nodo *args_node, char *func_label, HashTableStack *stack);
void capture_params(Nodo* header, Nodo* params);
void encapsulate_program_code(Nodo* program_node);
const char* get_operation_string(ilocOp operation);
void print_arg(ilocArg *arg);
void print_code(ilocCode *code);
int get_header_ar_size(Nodo *header);
ilocCode* gen_header_activation_register(Nodo* header);

#endif