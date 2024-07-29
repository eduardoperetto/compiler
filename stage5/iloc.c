#include "iloc.h"

#include "hash_table.h"

#define TEMP_PREFFIX "r"
#define LABEL_PREFFIX "L"

static int curr_temp_id = 0;
static int curr_label_id = 0;

static int curr_func_rbss = 0;

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

void mock_code(Nodo *node) {
  node->iloc_code = code_mock();
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

ilocArg *rpc_arg() {
  return build_arg_label("rpc");
}

ilocCode *end_function() {
  ilocCode *code;
  char *reg_return_addr = gen_temp();
  code = gen_code(LOADAI, rfp_arg(), build_arg_im_value(0), build_arg_label(reg_return_addr));  /// loadAI rfp, 0 => r0 ; obtém end. retorno
  char *reg_rsp = gen_temp();
  code = merge_code(code, gen_code(LOADAI, rfp_arg(), build_arg_im_value(4), build_arg_label(reg_rsp)));  // loadAI rfp, 4 => r1 //obtém rsp salvo
  char *reg_rfp = gen_temp();
  code = merge_code(code, gen_code(LOADAI, rfp_arg(), build_arg_im_value(8), build_arg_label(reg_rfp)));  // loadAI rfp, 8 => r2 //obtém rfp salvo
  code = merge_code(code, gen_code(I2I, build_arg_label(reg_rsp), rsp_arg(), NULL));                      // i2i r1 => rsp
  code = merge_code(code, gen_code(I2I, build_arg_label(reg_rfp), rfp_arg(), NULL));                      // i2i r2 => rfp

  code = merge_code(code, gen_code(JUMP, build_arg_label(reg_return_addr), NULL, NULL));  // jump => r0
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

  nop = merge_code(nop, header->iloc_code);

  header->iloc_code = merge_code(nop, body->iloc_code);

  update_func_label(table_stack, (header->valor_lexico).label, (nop->arg1)->label);

  if (!is_main) {
    header->iloc_code = merge_code(header->iloc_code, end_function());
  }
}

void capture_params(Nodo* header, Nodo* params) {
    if (params == NULL) return;
    int rbss = get_header_ar_size(header);
    curr_func_rbss = rbss;
    int curr_offset = 0;

    
    ilocArg *temp = gen_temp_as_arg();
    ilocCode* load = gen_code(LOADAI, rfp_arg(), build_arg_im_value(12 + curr_offset), temp);
    ilocCode* store = gen_code(STOREAI, temp, rfp_arg(), build_arg_im_value(rbss + curr_offset));

    ilocCode *result = load;
    result = merge_code(result, store);
    for (int i=0; i < params->num_filhos; i++) {
        temp = gen_temp_as_arg();
        load = gen_code(LOADAI, rfp_arg(), build_arg_im_value(12 + curr_offset), temp);
        store = gen_code(STOREAI, temp, rfp_arg(), build_arg_im_value(rbss + curr_offset));
        result = merge_code(result, load);
        result = merge_code(result, store);
    }
    header->iloc_code = result;
}

int get_header_ar_size(Nodo *header) {
    int size = 12;                    // addr_return; rsp; rfp
    size += get_last_table_offset();  // size of parameters and local vars
    size += get_size(header->tipo);   // size of return value
    return size;
}

// Ordem de elementos no RA: endereço de retorno, old_rsp, old_rfp, parametros, valor_retorno; depois, vem var_locais.
ilocCode *gen_header_activation_register(Nodo *header) {
  int size = get_header_ar_size(header);
  return gen_code(ADDI, rsp_arg(), build_arg_im_value(size), rsp_arg());
}

void gen_load_var(Nodo *var) {
  ilocArg *result = gen_temp_as_arg();
  int rfp_offset = is_inside_main() ? 0 : curr_func_rbss;
  ilocCode *load = gen_code(LOADAI, rfp_arg(), build_arg_im_value(rfp_offset + var->table_local_addr), result);
  var->iloc_code = merge_code(var->iloc_code, load);
  var->temp_reg = result->temp_reg;
}

void gen_load_literal(Nodo *val_node) {
  ilocArg *result = gen_temp_as_arg();
  ilocCode *load;
  // Fazer um switch no futuro
  load = gen_code(LOADI, build_arg_im_value((val_node->valor_lexico).valor.i_val), NULL, result);
  val_node->iloc_code = load;
  val_node->temp_reg = result->temp_reg;
}

void gen_assignment(Nodo *assign_node, Nodo *destiny, Nodo *expr) {
  ilocCode *store;

  int rfp_offset = is_inside_main() ? 0 : curr_func_rbss;
  store = gen_code(STOREAI, build_arg_temp(expr->temp_reg), rfp_arg(), build_arg_im_value(rfp_offset + destiny->table_local_addr));

  assign_node->iloc_code = expr->iloc_code;
  assign_node->iloc_code = merge_code(assign_node->iloc_code, store);
}

void gen_return(Nodo *return_node, Nodo *expr_node) {
  int rfp_offset = is_inside_main() ? 0 : 12;
  rfp_offset += get_last_table_offset();
  ilocCode *load_result_on_rsp = gen_code(STOREAI, build_arg_temp(expr_node->temp_reg), rfp_arg(), build_arg_im_value(rfp_offset));
  return_node->iloc_code = merge_code(expr_node->iloc_code, load_result_on_rsp);
}

bool is_equal(char* str1, char* str2) {
    return strcmp(str1, str2) == 0;
}

void gen_comp_equal(Nodo *root, Nodo *arg1, Nodo *arg2) {
    mock_code(root);
}

void gen_comp_unequal(Nodo *root, Nodo *arg1, Nodo *arg2) {
    mock_code(root);
}

void gen_comp_less(Nodo *root, Nodo *arg1, Nodo *arg2) {
    mock_code(root);
}

void gen_comp_less_equal(Nodo *root, Nodo *arg1, Nodo *arg2) {
    mock_code(root);
}

void gen_comp_greater(Nodo *root, Nodo *arg1, Nodo *arg2) {
    mock_code(root);
}

void gen_comp_greater_equal(Nodo *root, Nodo *arg1, Nodo *arg2) {
    mock_code(root);
}

void gen_sum(Nodo *root, Nodo *arg1, Nodo *arg2) {
    ilocArg *result_reg = gen_temp_as_arg();
    ilocCode *code = arg1->iloc_code;
    code = merge_code(code, arg2->iloc_code);
    ilocCode *sum = gen_code(ADD, build_arg_temp(arg1->temp_reg), build_arg_temp(arg2->temp_reg), result_reg);
    code = merge_code(code, sum);
    root->iloc_code = code;
    root->temp_reg = result_reg->temp_reg;
}

void gen_diff(Nodo *root, Nodo *arg1, Nodo *arg2) {
    mock_code(root);
}

void gen_mult(Nodo *root, Nodo *arg1, Nodo *arg2) {
    mock_code(root);
}

void gen_div(Nodo *root, Nodo *arg1, Nodo *arg2) {
    mock_code(root);
}

void gen_bin_expr(Nodo *root, Nodo *arg1, Nodo *arg2) {
    char *operator = (root->valor_lexico).label;
    if (is_equal(operator, "==")) {
        gen_comp_equal(root, arg1, arg2);
    } else if (is_equal(operator, "!=")) {
        gen_comp_unequal(root, arg1, arg2);
    } else if (is_equal(operator, "<")) {
        gen_comp_less(root, arg1, arg2);
    } else if (is_equal(operator, "<=")) {
        gen_comp_less_equal(root, arg1, arg2);
    } else if (is_equal(operator, ">")) {
        gen_comp_greater(root, arg1, arg2);
    } else if (is_equal(operator, ">=")) {
        gen_comp_greater_equal(root, arg1, arg2);
    } else if (is_equal(operator, "+")) {
        gen_sum(root, arg1, arg2);
    } else if (is_equal(operator, "-")) {
        gen_diff(root, arg1, arg2);
    } else if (is_equal(operator, "*")) {
        gen_mult(root, arg1, arg2);
    } else if (is_equal(operator, "/")) {
        gen_div(root, arg1, arg2);
    } else {
        mock_code(root);
    }
}

void gen_unit_expr(Nodo *root, Nodo *arg1) {
  mock_code(root);
}

void gen_while(Nodo *root_while) {
  mock_code(root_while);
}

void gen_if(Nodo *root_if) {
  mock_code(root_if);
}

void gen_call_func(Nodo *call_node, Nodo *args_node, char *func_label, HashTableStack *stack) {
  int return_addr = -1;
  char *reg_return_addr = gen_temp();
  ilocCode *calc_return_addr = gen_code(ADDI, rpc_arg(), build_arg_im_value(4), build_arg_label(reg_return_addr));
  ilocCode *store_addr = gen_code(STOREAI, build_arg_label(reg_return_addr), rsp_arg(), build_arg_im_value(0));
  ilocCode *store_rsp = gen_code(STOREAI, rsp_arg(), rsp_arg(), build_arg_im_value(4));
  ilocCode *store_rfp = gen_code(STOREAI, rfp_arg(), rsp_arg(), build_arg_im_value(8));

  ilocCode *result_code = calc_return_addr;
  result_code = merge_code(result_code, store_addr);
  result_code = merge_code(result_code, store_rsp);
  result_code = merge_code(result_code, store_rfp);

  if (args_node != NULL) {
    int offset = 12;
    ilocCode *storeArg = args_node->iloc_code;
    storeArg = merge_code(storeArg, gen_code(STOREAI, build_arg_temp(args_node->temp_reg), rsp_arg(), build_arg_im_value(offset)));
    result_code = merge_code(result_code, storeArg);

    for (int i=0; args_node->num_filhos; i++) {
        Nodo* current = args_node->filhos[i];
        offset += get_size(current->tipo);
        storeArg = current->iloc_code;
        storeArg = merge_code(storeArg, gen_code(STOREAI, build_arg_temp(args_node->temp_reg), rsp_arg(), build_arg_im_value(offset)));
        result_code = merge_code(result_code, storeArg);
    }
  }
  
  ilocCode *jump = gen_code(JUMPI, build_arg_label(func_label), NULL, NULL);
  char *reg_result = gen_temp();
  ilocCode *load_result = gen_code(LOADI, rsp_arg(), build_arg_im_value(12), build_arg_temp(reg_result));

  result_code = merge_code(result_code, jump);
  result_code = merge_code(result_code, load_result);

  call_node->iloc_code = result_code;
  call_node->temp_reg = reg_result;
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
    switch (current->operation) {
      case NOP:
        printf("%s: NOP\n", (current->arg1)->label);
        break;

      case STOREAI:
        if ((current->arg1)->temp_reg) {
          printf("STOREAI %s => %s, %d\n",
                 (current->arg1)->temp_reg,
                 (current->arg2)->label,
                 (current->arg3)->imediate_value);
        } else {
          printf("STOREAI %s => %s, %d\n",
                 (current->arg1)->label,
                 (current->arg2)->label,
                 (current->arg3)->imediate_value);
        }
        break;

      case HALT:
        printf("HALT\n");
        return;

      default:
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
        break;
    }
    current = current->next;
  }
}
