#include "assembly.h"
#include "hash_table.h"
#include "string.h"

#define TEMP_PREFFIX "r"
#define LABEL_PREFFIX "L"

static int curr_temp_id = 0;
static int curr_label_id = 0;
static int curr_func_rbss = 0;

static asmArg *main_label;

static Identifier *globals;

void save_global(Identifier *id) {
    if (globals == NULL) {
        globals = id;
    } else {
        Identifier *current = globals;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = id;
    }
    id->next = NULL;
}

char *gen_label() {
  int length = snprintf(NULL, 0, "%s%d", LABEL_PREFFIX, curr_label_id);
  char *label = (char *)malloc(length + 1);
  snprintf(label, length + 1, "%s%d", LABEL_PREFFIX, curr_label_id);
  curr_label_id++;
  return label;
}

char *gen_temp() {
    const char *registers[] = {"%eax", "%ebx", "%ecx", "%edx"};
    int register_index = curr_temp_id % 4;
    curr_temp_id++;

    char *temp = (char *)malloc(strlen(registers[register_index]) + 1);
    strcpy(temp, registers[register_index]);

    return temp;
}

asmCode *gen_code(asmOp operation, asmArg *arg1, asmArg *arg2, asmArg *arg3) {
  asmCode *code = malloc(sizeof(asmCode));
  code->operation = operation;
  code->arg1 = arg1;
  code->arg2 = arg2;
  code->arg3 = arg3;
  code->next = NULL;
  return code;
}

asmCode *halt() {
  gen_code(HALT, NULL, NULL, NULL);
}

asmCode *code_mock() {
  return gen_code(NOP, build_arg_label("[To be done]"), NULL, NULL);
}

void mock_code(Nodo *node) {
  node->asm_code = code_mock();
}

asmArg *build_arg_im_value(int im_value) {
  asmArg *arg = malloc(sizeof(asmArg));
  arg->imediate_value = im_value;
  arg->label = NULL;
  arg->temp_reg = NULL;
  return arg;
}

asmArg *build_arg_label(char *label) {
  asmArg *arg = malloc(sizeof(asmArg));
  arg->imediate_value = -1;
  arg->label = label;
  arg->temp_reg = NULL;
  return arg;
}

asmArg *build_arg_temp(char *temp_reg) {
  asmArg *arg = malloc(sizeof(asmArg));
  arg->imediate_value = -1;
  arg->label = NULL;
  arg->temp_reg = temp_reg;
  return arg;
}

asmCode *findLastNode(asmCode *code_list) {
  if (code_list == NULL) return NULL;

  asmCode *currentCode = code_list;
  while (currentCode->next != NULL) {
    currentCode = currentCode->next;
  }
  return currentCode;
}

asmCode *merge_code(asmCode *current, asmCode *new) {
  if (current == NULL) return new;
  if (new == NULL) return current;

  asmCode *lastNodeInNewList = findLastNode(current);
  lastNodeInNewList->next = new;
  return current;
}

void append_node_codes(Nodo *destiny, Nodo *appending) {
  destiny->asm_code = merge_code(destiny->asm_code, appending->asm_code);
}

asmCode *gen_label_with_nop() {
  char *label = gen_label();
  return gen_code(NOP, build_arg_label(label), NULL, NULL);
}

asmArg *gen_temp_as_arg() {
  char *reg = gen_temp();
  return build_arg_temp(reg);
}

char *global_ref(char* var_name) {
  int size = strlen(var_name) + strlen("(%%rip)") + 1;
  char *result = (char *)malloc(size);
  snprintf(result, size, "%s(%%rip)", var_name);
  return result;
}

char *local_ref(Nodo* var) {
  int size = 3 + strlen("(%%rbp)") + 1;
  char *result = (char *)malloc(size);
  snprintf(result, size, "-%d(%%rbp)", var->table_local_addr + get_size(var->tipo));
  return result;
}

asmArg *rfp_arg() {
  return build_arg_label("%rfp");
}

asmArg *rbss_arg() {
  return build_arg_label("%rbss");
}

asmArg *rsp_arg() {
  return build_arg_label("%rsp");
}

asmArg *rpc_arg() {
  return build_arg_label("%rpc");
}

asmArg *rbp_arg() {
  return build_arg_label("%rbp");
}

asmArg *eax_arg() {
  return build_arg_temp("%eax");
}

asmCode *end_function() {
  asmCode *code;
  code = gen_code(POPQ, rbp_arg(), NULL, NULL);
  code = merge_code(code, gen_code(RET, NULL, NULL, NULL));
  return code;
}

void gen_func_declaration(Nodo *header, Nodo *body, HashTableStack *table_stack) {
  asmCode *nop;
  bool is_main = strcmp("main", (header->valor_lexico).label) == 0;
  if (is_main) {
    main_label = build_arg_label("main");
    nop = gen_code(NOP, main_label, NULL, NULL);
  } else {
    nop = gen_code(NOP, build_arg_label((header->valor_lexico).label), NULL, NULL);
  }

  nop = merge_code(nop, gen_code(PUSHQ, rbp_arg(), NULL, NULL));
  nop = merge_code(nop, gen_code(MOVQ, rsp_arg(), rbp_arg(), NULL));

  nop = merge_code(nop, header->asm_code);

  header->asm_code = merge_code(nop, body->asm_code);

  update_func_label(table_stack, (header->valor_lexico).label, (nop->arg1)->label);

  header->asm_code = merge_code(header->asm_code, end_function());
}

void capture_params(Nodo *header, Nodo *params) {
  if (params == NULL) return;
  int rbss = get_header_ar_size(header);
  curr_func_rbss = rbss;
  int curr_offset = 0;

  asmArg *temp = gen_temp_as_arg();
  asmCode *load = gen_code(LOADAI, rfp_arg(), build_arg_im_value(12 + curr_offset), temp);
  asmCode *store = gen_code(STOREAI, temp, rfp_arg(), build_arg_im_value(rbss + curr_offset));

  asmCode *result = load;
  result = merge_code(result, store);
  for (int i = 0; i < params->num_filhos; i++) {
    temp = gen_temp_as_arg();
    load = gen_code(LOADAI, rfp_arg(), build_arg_im_value(12 + curr_offset), temp);
    store = gen_code(STOREAI, temp, rfp_arg(), build_arg_im_value(rbss + curr_offset));
    result = merge_code(result, load);
    result = merge_code(result, store);
  }
  header->asm_code = result;
}

int get_header_ar_size(Nodo *header) {
  int size = 12;                    // addr_return; rsp; rfp
  size += get_last_table_offset();  // size of parameters and local vars
  size += get_size(header->tipo);   // size of return value
  return size;
}

// Ordem de elementos no RA: endereço de retorno, old_rsp, old_rfp, parametros, valor_retorno; depois, vem var_locais.
asmCode *gen_header_activation_register(Nodo *header) {
  int size = get_header_ar_size(header);
  return gen_code(ADDL, rsp_arg(), build_arg_im_value(size), rsp_arg());
}

void gen_load_var(Nodo *var, bool is_global) {
  if (is_global) {
    var->temp_reg = global_ref((var->valor_lexico).label);
    return;
  }
  var->temp_reg = local_ref(var);
}

void gen_load_literal(Nodo *val_node) {
  asmArg *result = gen_temp_as_arg();
  asmCode *load;
  // Fazer um switch no futuro
  load = gen_code(MOVL, build_arg_im_value((val_node->valor_lexico).valor.i_val), result, NULL);
  val_node->asm_code = load;
  val_node->temp_reg = result->temp_reg;
}

void gen_assignment(Nodo *assign_node, Nodo *destiny, bool is_global, Nodo *expr) {
  asmCode *store;

  asmArg *storageReg = is_global ? build_arg_temp(global_ref((destiny->valor_lexico).label)) : build_arg_temp(local_ref(destiny));

  store = gen_code(MOVL, build_arg_temp(expr->temp_reg), storageReg, NULL);

  assign_node->asm_code = expr->asm_code;
  assign_node->asm_code = merge_code(assign_node->asm_code, store);
}

void gen_return(Nodo *return_node, Nodo *expr_node) {
  return_node->asm_code = expr_node->asm_code;
  if (strcmp(expr_node->temp_reg, "%eax") != 0) {
    return_node->asm_code = merge_code(return_node->asm_code, gen_code(MOVL, build_arg_temp(expr_node->temp_reg), eax_arg(), NULL));
  }
  return_node->asm_code = merge_code(return_node->asm_code, gen_code(POPQ, rbp_arg(), NULL, NULL));
  return_node->asm_code = merge_code(return_node->asm_code, gen_code(RET, NULL, NULL, NULL));
}

bool is_equal(char *str1, char *str2) {
  return strcmp(str1, str2) == 0;
}

void gen_div(asmOp op, Nodo *root, Nodo *arg1, Nodo *arg2) {
  asmCode *code = arg1->asm_code;
  code = merge_code(code, arg2->asm_code);
  asmArg *temp = eax_arg();
  code = merge_code(code, gen_code(MOVL, build_arg_temp(arg1->temp_reg), temp, NULL));
  code = merge_code(code, gen_code(CLTD, NULL, NULL, NULL));
  code = merge_code(code, gen_code(IDIVL, build_arg_temp(arg2->temp_reg), NULL, NULL));
  root->asm_code = code;
  root->temp_reg = temp->temp_reg;
}

void gen_bin_expr_from_op(asmOp op, Nodo *root, Nodo *arg1, Nodo *arg2) {
  if (op == DIV) {
    gen_div(op, root, arg1, arg2);
    return;
  }
  asmCode *code = arg1->asm_code;
  code = merge_code(code, arg2->asm_code);
  asmArg *temp = eax_arg();
  code = merge_code(code, gen_code(MOVL, build_arg_temp(arg1->temp_reg), temp, NULL));
  asmCode *operation = gen_code(op, build_arg_temp(arg2->temp_reg), temp, NULL);
  code = merge_code(code, operation);
  root->asm_code = code;
  root->temp_reg = temp->temp_reg;
}

asmOp string_to_op(char *operator) {
  if (is_equal(operator, "==")) {
    return CMPL;
  } else if (is_equal(operator, "!=")) {
    return CMPL;
  } else if (is_equal(operator, "<")) {
    return CMPL;
  } else if (is_equal(operator, "<=")) {
    return CMPL;
  } else if (is_equal(operator, ">")) {
    return CMPL;
  } else if (is_equal(operator, ">=")) {
    return CMPL;
  } else if (is_equal(operator, "+")) {
    return ADDL;
  } else if (is_equal(operator, "-")) {
    return SUBL;
  } else if (is_equal(operator, "*")) {
    return MULT;
  } else if (is_equal(operator, "/")) {
    return DIV;
  } else if (is_equal(operator, "&")) {
    return AND;
  } else if (is_equal(operator, "|")) {
    return OR;
  } else {
    return NOP;
  }
}

void gen_bin_expr(Nodo *root, Nodo *arg1, Nodo *arg2) {
  char *operator=(root->valor_lexico).label;
  gen_bin_expr_from_op(string_to_op(operator), root, arg1, arg2);
}

void gen_invert_signal(Nodo *root, Nodo *arg) {
  asmArg *result_reg = gen_temp_as_arg();
  asmCode *code = arg->asm_code;
  asmCode *operation = gen_code(MULTI, build_arg_temp(arg->temp_reg), build_arg_im_value(-1), result_reg);
  code = merge_code(code, operation);
  root->asm_code = code;
  root->temp_reg = result_reg->temp_reg;
}

void gen_logic_invert(Nodo *root, Nodo *arg) {
  asmArg *result_reg = gen_temp_as_arg();
  asmCode *code = arg->asm_code;
  asmCode *operation = gen_code(CMP_EQ, build_arg_temp(arg->temp_reg), build_arg_im_value(0), result_reg);
  code = merge_code(code, operation);
  root->asm_code = code;
  root->temp_reg = result_reg->temp_reg;
}

void gen_while(Nodo *root_while, Nodo *expr, Nodo *block) {
  mock_code(root_while);
  asmArg *arg_aux = build_arg_label(gen_label());
  asmArg *arg_label_true = build_arg_label(gen_label());
  asmArg *arg_label_false = build_arg_label(gen_label());

  asmCode *while_code = gen_code(NOP, arg_aux, NULL, NULL); // Label aux
  while_code = merge_code(while_code, expr->asm_code); // Expr.code
  while_code = merge_code(while_code, gen_code(JNE, arg_label_true, NULL, NULL)); // Branch based on expr temp
  while_code = merge_code(while_code, gen_code(JUMP, arg_label_false, NULL, NULL)); // Branch based on expr temp
  while_code = merge_code(while_code, gen_code(NOP, arg_label_true, NULL, NULL)); // Label true
  while_code = merge_code(while_code, block->asm_code); // Cmd block
  while_code = merge_code(while_code, gen_code(JUMPI, arg_aux, NULL, NULL)); // Jump aux
  while_code = merge_code(while_code, gen_code(NOP, arg_label_false, NULL, NULL)); // Label false (end)

  root_while->asm_code = while_code;
}

void gen_if(Nodo *root_if, Nodo *expr, Nodo *true_block, Nodo *else_block) {
  char *label_true = gen_label();
  asmCode *label_nop_true = gen_code(NOP, build_arg_label(label_true), NULL, NULL);
  char *label_false = gen_label();
  asmCode *label_nop_false = gen_code(NOP, build_arg_label(label_false), NULL, NULL);

  char *label_jump_false = gen_label();
  asmCode *label_nop_jump_false = gen_code(NOP, build_arg_label(label_jump_false), NULL, NULL);

  asmCode *result_code = expr->asm_code;

  asmCode *branch = gen_code(JNE, build_arg_label(label_true), NULL, NULL);
  result_code = merge_code(result_code, branch);

  result_code = merge_code(result_code, gen_code(JUMP, build_arg_label(label_false), NULL, NULL));

  result_code = merge_code(result_code, label_nop_true);
  result_code = merge_code(result_code, true_block->asm_code);
  if (else_block != NULL) {
    result_code = merge_code(result_code, gen_code(JUMPI, build_arg_label(label_jump_false), NULL, NULL));
  }
  result_code = merge_code(result_code, label_nop_false);
  if (else_block != NULL) {
    result_code = merge_code(result_code, else_block->asm_code);
    result_code = merge_code(result_code, label_nop_jump_false);
  }
  root_if->asm_code = result_code;
}

void pass_parameters(asmCode *destiny, Nodo* args_node)
{
 if (args_node != NULL) {
    int offset = 12;
    asmCode *storeArg = args_node->asm_code;
    storeArg = merge_code(storeArg, gen_code(STOREAI, build_arg_temp(args_node->temp_reg), rsp_arg(), build_arg_im_value(offset)));
    destiny = merge_code(destiny, storeArg);

    for (int i = 0; args_node->num_filhos; i++) {
      Nodo *current = args_node->filhos[i];
      offset += get_size(current->tipo);
      storeArg = current->asm_code;
      storeArg = merge_code(storeArg, gen_code(STOREAI, build_arg_temp(args_node->temp_reg), rsp_arg(), build_arg_im_value(offset)));
      destiny = merge_code(destiny, storeArg);
    }
  }
}

void gen_call_func(Nodo *call_node, Nodo *args_node, char *func_label, HashTableStack *stack) {
  asmCode *result_code = gen_code(CALL, build_arg_label(func_label), NULL, NULL);

  // pass_parameters(result_code, args_node);
  call_node->asm_code = gen_code(MOVL, build_arg_im_value(0), eax_arg(), NULL);
  call_node->asm_code = merge_code(call_node->asm_code, result_code);
  call_node->temp_reg = "%eax";
}

int calc_rbss(Nodo *program_node) {
  asmCode *current = program_node->asm_code;
  int code_count = 1;
  while (current->next != NULL) {
    code_count += 1;
    current = current->next;
  }
  return code_count + 6;  // encapsulation adds 5 instructions
}

asmCode* gen_program_prologue(HashTable* global_table) {
    int buffer_size = 1024; // To do: calculate size
    char* prologue = (char*)malloc(buffer_size);
    if (!prologue) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // Initialize the prologue string
    snprintf(prologue, buffer_size, ".file \"%s\"\n\t", ""); // Can we get the filename?

    Identifier* globalVars;
    Identifier* globalFuncs;
    getAllIds(global_table, &globalVars, &globalFuncs);

    // Setup global variables/functions
    Identifier* current = globalVars;
    while (current != NULL) {
        if (current->isGlobal) {
          snprintf(
            prologue + strlen(prologue), 
            buffer_size - strlen(prologue),
            ".text\n\t.globl %s\n\t.bss\n\t.align 4\n\t.type %s, @object\n\t.size %s, %d\n%s:\n\t.zero %d\n\t",
            current->name, 
            current->name, 
            current->name,
            get_size(current->type),
            current->name,
            get_size(current->type)
          );
        }
        current = current->next;
    }

    current = globalFuncs;
    while (current != NULL) {
        if (current->isGlobal) {
          snprintf(
            prologue + strlen(prologue), buffer_size - strlen(prologue),
              ".text\n\t.globl %s\n\t.type %s, @function\n\t", 
              current->name, 
              current->name
            );
        }
        current = current->next;
    }

    return gen_code(ASM_CHAR_CODE, build_arg_label(prologue), NULL, NULL);
}

void encapsulate_program_code(Nodo *program_node, HashTable *global_table) {
  if (!main_label) {
    printErrorPrefix(get_line_number());
    printf("Programa deve conter uma função 'main'.\n");
    exit(-1);
  }
  asmCode *init;
  init = gen_program_prologue(global_table);

  // Program encapsulated
  program_node->asm_code = merge_code(init, program_node->asm_code);
  //
}

/* Debug */
const char *get_operation_string(asmOp operation) {
  switch (operation) {
    case NOP:
      return "nop";
    case ADD:
      return "add";
    case SUB:
      return "sub";
    case MULT:
      return "imull";
    case DIV:
      return "div";
    case ADDL:
      return "addl";
    case SUBL:
      return "subl";
    case RSUBL:
      return "rsubl";
    case MULTI:
      return "multl";
    case DIVI:
      return "divl";
    case RDIVI:
      return "rdivl";
    case LSHIFT:
      return "lshift";
    case LSHIFTI:
      return "lshiftl";
    case RSHIFT:
      return "rshift";
    case RSHIFTI:
      return "rshiftl";
    case AND:
      return "and";
    case ANDI:
      return "andl";
    case OR:
      return "or";
    case ORI:
      return "orl";
    case XOR:
      return "xor";
    case XORI:
      return "xorl";
    case MOVL:
      return "movl";
    case LOAD:
      return "load";
    case LOADAI:
      return "loadAl";
    case LOADA0:
      return "loadA0";
    case CLOAD:
      return "cload";
    case CLOADAI:
      return "cloadAl";
    case CLOADA0:
      return "cloadA0";
    case STORE:
      return "store";
    case STOREAI:
      return "storeAl";
    case STOREAO:
      return "storeAO";
    case CSTORE:
      return "cstore";
    case CSTOREAI:
      return "cstoreAl";
    case CSTOREAO:
      return "cstoreAO";
    case I2I:
      return "i2i";
    case C2C:
      return "c2c";
    case C2I:
      return "c2i";
    case I2C:
      return "i2c";
    case JUMPI:
      return "jmp";
    case JUMP:
      return "jmp";
    case CBR:
      return "cbr";
    case CMP_LT:
      return "cmp_LT";
    case CMP_LE:
      return "cmp_LE";
    case CMP_EQ:
      return "cmp_EQ";
    case CMP_GE:
      return "cmp_GE";
    case CMP_GT:
      return "cmp_GT";
    case CMP_NE:
      return "cmp_NE";
    case RET:
      return "ret";
    case PUSHQ:
      return "pushq";
    case MOVQ:
      return "movq";
    case POPQ:
      return "popq";
    case CALL:
      return "call";
    case CLTD:
      return "cltd";
    case IDIVL:
      return "idivl";
    case JE:
      return "je";
    case JNE:
      return "jne";
    case CMPL:
      return "cmpl";
    default:
      return "unknown_op";
  }
}

void print_arg(asmArg *arg) {
  if (arg == NULL) return;
  if (arg->temp_reg != NULL) {
    printf("%s", arg->temp_reg);
  } else if (arg->label != NULL) {
    printf("%s", arg->label);
  } else {
    printf("$%d", arg->imediate_value);
  }
}

void print_code(asmCode *code) {
  asmCode *current = code;

  if (current == NULL) {
    printf("Node has Null code\n");
    return;
  }

  while (current != NULL) {
    switch (current->operation) {
      case ASM_CHAR_CODE:
        printf("\t%s", (current->arg1)->label);
        break;

      case NOP:
        printf("\n%s: nop\n", (current->arg1)->label);
        break;
      
      case HALT:
        printf("\thalt\n");
        return;

      case RET:
        printf("\tret\n");
        break;

      default: // Default is: "OP arg1, arg2 => arg3" | "OP arg1 => arg2"
        printf("\t%s ", get_operation_string(current->operation));
        print_arg(current->arg1);
        if (current->arg2 != NULL) {
          printf(", ");
          print_arg(current->arg2);
        }
        if (current->arg3 != NULL) {
          printf(", ");
          print_arg(current->arg3);
        }
        printf("\n");
        break;
    }
    current = current->next;
  }
}
