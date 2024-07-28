#include "iloc.h"

#include "hash_table.h"

#define TEMP_PREFFIX "r"
#define LABEL_PREFFIX "L"

static int curr_temp_id = 0;
static int curr_label_id = 0;

static ilocArg *main_label;

char *gen_label() {
  int length = snprintf(NULL, 0, "%s%d", LABEL_PREFFIX, curr_label_id);
  char *label = (char *)malloc(length + 1);
  snprintf(label, length + 1, "%s%d", LABEL_PREFFIX, curr_label_id);
  curr_label_id++;
  return label;
}

char *gen_temp() {
  int length = snprintf(NULL, 0, "%s%d", TEMP_PREFFIX, curr_temp_id);
  char *temp = (char *)malloc(length + 1);
  snprintf(temp, length + 1, "%s%d", TEMP_PREFFIX, curr_temp_id);
  curr_temp_id++;
  return temp;
}

ilocCode *gen_code(ilocOp operation, ilocArg *arg1, ilocArg *arg2, ilocArg *arg3) {
  ilocCode *code = malloc(sizeof(ilocCode));
  code->operation = operation;
  code->arg1 = arg1;
  code->arg2 = arg2;
  code->arg3 = arg3;
  code->next = NULL;
  return code;
}

ilocCode *halt() {
  gen_code(HALT, NULL, NULL, NULL);
}

ilocCode *code_mock() {
  return gen_code(NOP, build_arg_label("[To be done]"), NULL, NULL);
}

ilocArg *build_arg_im_value(int im_value) {
  ilocArg *arg = malloc(sizeof(ilocArg));
  arg->imediate_value = im_value;
  arg->label = NULL;
  arg->temp_reg = NULL;
  return arg;
}

ilocArg *build_arg_label(char *label) {
  ilocArg *arg = malloc(sizeof(ilocArg));
  arg->imediate_value = -1;
  arg->label = label;
  arg->temp_reg = NULL;
  return arg;
}

ilocArg *build_arg_temp(char *temp_reg) {
  ilocArg *arg = malloc(sizeof(ilocArg));
  arg->imediate_value = -1;
  arg->label = NULL;
  arg->temp_reg = temp_reg;
  return arg;
}

ilocCode *findLastNode(ilocCode *code_list) {
  if (code_list == NULL) return NULL;

  ilocCode *currentCode = code_list;
  while (currentCode->next != NULL) {
    currentCode = currentCode->next;
  }
  return currentCode;
}

ilocCode *merge_code(ilocCode *current, ilocCode *new) {
  if (current == NULL) return new;
  if (new == NULL) return current;

  ilocCode *lastNodeInNewList = findLastNode(current);
  lastNodeInNewList->next = new;
  return current;
}

void append_node_codes(Nodo *destiny, Nodo *appending) {
  printf("Appending: ");
  print_node_code(appending);
  destiny->iloc_code = merge_code(destiny->iloc_code, appending->iloc_code);
  print_node_code(destiny);
}

ilocCode *gen_label_with_nop() {
  char *label = gen_label();
  return gen_code(NOP, build_arg_label(label), NULL, NULL);
}

ilocArg *gen_temp_as_arg() {
  char *reg = gen_temp();
  return build_arg_temp(reg);
}

ilocArg *rfp_arg() {
  return build_arg_label("rfp");
}

ilocArg *rbss_arg() {
  return build_arg_label("rbss");
}

ilocArg *rsp_arg() {
  return build_arg_label("rsp");
}

ilocCode *end_function() {
    ilocCode *code;
    char *reg_return_addr = gen_temp();
    code = gen_code(LOADAI, rfp_arg(), build_arg_im_value(0), build_arg_label(reg_return_addr)); /// loadAI rfp, 0 => r0 ; obtém end. retorno
    char *reg_rsp = gen_temp();
    code = merge_code(code, gen_code(LOADAI, rfp_arg(), build_arg_im_value(4), build_arg_label(reg_rsp))); // loadAI rfp, 4 => r1 //obtém rsp salvo
    char *reg_rfp = gen_temp();
    code = merge_code(code, gen_code(LOADAI, rfp_arg(), build_arg_im_value(8), build_arg_label(reg_rfp))); // loadAI rfp, 8 => r2 //obtém rfp salvo
    code = merge_code(code, gen_code(I2I, build_arg_label(reg_rsp), rsp_arg(), NULL)); // i2i r1 => rsp
    code = merge_code(code, gen_code(I2I, build_arg_label(reg_rfp), rfp_arg(), NULL)); // i2i r2 => rfp

    code = merge_code(code, gen_code(JUMP, build_arg_label(reg_return_addr), NULL, NULL)); // jump => r0
    return code;
}

void gen_func_declaration(Nodo *header, Nodo *body, HashTableStack *table_stack) {
  ilocCode *nop = gen_label_with_nop();

  ilocCode *rsp_update;

  bool is_main = strcmp("main", (header->valor_lexico).label) == 0;
  if (is_main) {
    main_label = nop->arg1;
    rsp_update = gen_code(ADDI, rsp_arg(), build_arg_im_value(get_last_table_offset()), rsp_arg());
  } else {
    nop = merge_code(nop, gen_code(I2I, rsp_arg(), rfp_arg(), NULL));
    rsp_update = gen_header_activation_register(header);
  }

  nop = merge_code(nop, rsp_update);

  header->iloc_code = merge_code(nop, body->iloc_code);

  update_func_label(table_stack, (header->valor_lexico).label, (nop->arg1)->label);

    if (!is_main) {
        header->iloc_code = merge_code(header->iloc_code, end_function());
    }

  print_code(header->iloc_code);
}

// Ordem de elementos no RA: endereço de retorno, old_rsp, old_rfp, parametros, valor_retorno; depois, vem var_locais.
ilocCode* gen_header_activation_register(Nodo* header) {
    int size = 12; // addr_return; rsp; rfp
    size += get_last_table_offset(); // size of parameters and local vars
    size += get_size(header->tipo); // size of return value
    return gen_code(ADDI, rsp_arg(), build_arg_im_value(size), rsp_arg());
}

void gen_load_var(Nodo *var) {
  ilocArg *result = gen_temp_as_arg();
  ilocCode *load = gen_code(LOADAI, rfp_arg(), build_arg_im_value(var->table_local_addr), result);
  var->iloc_code = merge_code(var->iloc_code, load);
  var->temp_reg = result->temp_reg;
  print_code(var->iloc_code);
}

void gen_load_literal(Nodo *val_node) {
  ilocArg *result = gen_temp_as_arg();
  ilocCode *load;
  // Fazer um switch no futuro
  load = gen_code(LOADI, build_arg_im_value((val_node->valor_lexico).valor.i_val), NULL, result);
  val_node->iloc_code = load;
  val_node->temp_reg = result->temp_reg;
  print_code(val_node->iloc_code);
}

void gen_assignment(Nodo *assign_node, Nodo *arg3, Nodo *expr) {
  ilocCode *store;

  store = gen_code(STOREAI, build_arg_temp(expr->temp_reg), rfp_arg(), build_arg_im_value(arg3->table_local_addr));

  assign_node->iloc_code = expr->iloc_code;
  merge_code(assign_node->iloc_code, store);
  print_code(assign_node->iloc_code);
}

void gen_return() {
}

int calc_rbss(Nodo *program_node) {
  ilocCode *current = program_node->iloc_code;
  int code_count = 1;
  while (current->next != NULL) {
    code_count += 1;
    current = current->next;
  }
  return code_count + 6;  // encapsulation adds 5 instructions
}

void encapsulate_program_code(Nodo *program_node) {
  if (!main_label) {
    printErrorPrefix(get_line_number());
    printf("Programa deve conter uma função 'main'.\n");
    exit(-1);
  }

  ilocCode *init;
  init = gen_code(LOADI, build_arg_im_value(1024), rsp_arg(), NULL);
  init = merge_code(init, gen_code(LOADI, build_arg_im_value(1024), rfp_arg(), NULL));
  init = merge_code(init, gen_code(LOADI, build_arg_im_value(calc_rbss(program_node)), rbss_arg(), NULL));
  ilocCode *jump_to_main = gen_code(JUMPI, main_label, NULL, NULL);
  init = merge_code(init, jump_to_main);
  program_node->iloc_code = merge_code(init, program_node->iloc_code);
  program_node->iloc_code = merge_code(program_node->iloc_code, halt());
}

/* Debug */
const char *get_operation_string(ilocOp operation) {
  switch (operation) {
    case NOP:
      return "NOP";
    case ADD:
      return "ADD";
    case SUB:
      return "SUB";
    case MULT:
      return "MULT";
    case DIV:
      return "DIV";
    case ADDI:
      return "ADDI";
    case SUBI:
      return "SUBI";
    case RSUBI:
      return "RSUBI";
    case MULTI:
      return "MULTI";
    case DIVI:
      return "DIVI";
    case RDIVI:
      return "RDIVI";
    case LSHIFT:
      return "LSHIFT";
    case LSHIFTI:
      return "LSHIFTI";
    case RSHIFT:
      return "RSHIFT";
    case RSHIFTI:
      return "RSHIFTI";
    case AND:
      return "AND";
    case ANDI:
      return "ANDI";
    case OR:
      return "OR";
    case ORI:
      return "ORI";
    case XOR:
      return "XOR";
    case XORI:
      return "XORI";
    case LOADI:
      return "LOADI";
    case LOAD:
      return "LOAD";
    case LOADAI:
      return "LOADAI";
    case LOADA0:
      return "LOADA0";
    case CLOAD:
      return "CLOAD";
    case CLOADAI:
      return "CLOADAI";
    case CLOADA0:
      return "CLOADA0";
    case STORE:
      return "STORE";
    case STOREAI:
      return "STOREAI";
    case STOREAO:
      return "STOREAO";
    case CSTORE:
      return "CSTORE";
    case CSTOREAI:
      return "CSTOREAI";
    case CSTOREAO:
      return "CSTOREAO";
    case I2I:
      return "I2I";
    case C2C:
      return "C2C";
    case C2I:
      return "C2I";
    case I2C:
      return "I2C";
    case JUMPI:
      return "JUMPI";
    case JUMP:
      return "JUMP";
    case CBR:
      return "CBR";
    case CMP_LT:
      return "CMP_LT";
    case CMP_LE:
      return "CMP_LE";
    case CMP_EQ:
      return "CMP_EQ";
    case CMP_GE:
      return "CMP_GE";
    case CMP_GT:
      return "CMP_GT";
    case CMP_NE:
      return "CMP_NE";
    default:
      return "UNKNOWN";
  }
}

void print_arg(ilocArg *arg) {
  if (arg == NULL) return;
  if (arg->temp_reg != NULL) {
    printf("%s", arg->temp_reg);
  } else if (arg->label != NULL) {
    printf("%s", arg->label);
  } else {
    printf("%d", arg->imediate_value);
  }
}

void print_code(ilocCode *code) {
  printf("------------------------------\n");
  ilocCode *current = code;
  if (current == NULL) {
    printf("Node has Null code\n");
    return;
  }
  while (current != NULL) {
    if (current->operation == NOP) {
      printf("%s: NOP\n", (current->arg1)->label);
      current = current->next;
      continue;
    }
    if (current->operation == STOREAI) {
      printf("STOREAI %s => %s, %d\n", (current->arg1)->temp_reg, (current->arg2)->label, (current->arg3)->imediate_value);
      current = current->next;
      continue;
    } else if (current->operation == HALT) {
      printf("HALT\n");
      return;
    }
    printf("%s ", get_operation_string(current->operation));
    print_arg(current->arg1);
    if (current->arg2 != NULL) {
      printf(", ");
      print_arg(current->arg2);
    }
    if (current->arg3 != NULL) {
      printf(" => ");
      print_arg(current->arg3);
    }
    printf("\n");
    current = current->next;
  }
}
