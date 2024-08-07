%{
/**
* Projeto Compiladores INF01147 2024/1
* Alunos: 
* Eduardo Raupp Peretto - 00313439
*
**/

#include "tree.h"
#include "hash_table.h"
#include "assembly.h"

extern HashTableStack* tableStack;
extern HashTable* globalTable;

int yylex(void);
void yyerror (char const *mensagem);
int get_line_number(void);
extern void *tree;

#define YYDEBUG 1

void prt_dbg(char* rule) {
	#if DEBUG_PARSER
	printf("%s\n", rule);
	#endif
}

%}

%define parse.error verbose

%union {
   valorLexico valor_lexico;
   struct Nodo *nodo;
   TipoToken tipo;
   struct Identifier *identifier;
}

%token TK_PR_INT        
%token TK_PR_FLOAT      
%token TK_PR_BOOL       
%token TK_PR_IF         
%token TK_PR_ELSE       
%token TK_PR_WHILE      
%token TK_PR_RETURN     

%token <valor_lexico> TK_OC_LE         
%token <valor_lexico> TK_OC_GE         
%token <valor_lexico> TK_OC_EQ         
%token <valor_lexico> TK_OC_NE         
%token <valor_lexico> TK_OC_AND        
%token <valor_lexico> TK_OC_OR         

%token TK_ERRO   

%token <valor_lexico> TK_IDENTIFICADOR  
%token <valor_lexico> TK_LIT_INT       
%token <valor_lexico> TK_LIT_FLOAT      
%token <valor_lexico> TK_LIT_FALSE      
%token <valor_lexico> TK_LIT_TRUE       
%token <valor_lexico> '+' '-' '|' '*' '/' '<' '>' '=' '!' '&' '%'

%type <valor_lexico> operador_bin_prec5
%type <valor_lexico> operador_bin_prec4
%type <valor_lexico> operador_bin_prec3
%type <valor_lexico> operador_bin_prec2

%type <nodo> literal
%type <nodo> expressao_terminal
%type <nodo> expressao9
%type <nodo> expressao8
%type <nodo> expressao7
%type <nodo> expressao6
%type <nodo> expressao5
%type <nodo> expressao4
%type <nodo> expressao3
%type <nodo> expressao2
%type <nodo> expressao
%type <nodo> argumentos_funcao
%type <nodo> lista_argumentos_funcao
%type <nodo> argumento_funcao
%type <nodo> bloco_while
%type <nodo> estrutura_condicional
%type <nodo> retorno_funcao
%type <nodo> especificacao_variaveis_internas
%type <nodo> declaracao_variavel_interna
%type <nodo> invocacao_funcao
%type <nodo> atribuicao
%type <nodo> instrucao_simples
%type <nodo> sequencia_comandos
%type <nodo> bloco_instrucoes_funcao
%type <nodo> bloco_instrucoes
%type <nodo> corpo_funcao
%type <nodo> lista_parametros
%type <nodo> parametro
%type <nodo> cabecalho_funcao
%type <nodo> definicao_de_funcao
%type <nodo> especificacao_variaveis
%type <nodo> declaracao_variavel_externa
%type <nodo> definicao_global
%type <nodo> definicoes_globais
%type <nodo> programa

%type <identifier> variavel

%type <tipo> tipo

%%

programa: definicoes_globais { 
	tree = $$;
	prt_dbg("programa"); 
	encapsulate_program_code($$);
	#if DEBUG_PARSER
	HashTable* topTable = getTop(&tableStack);
	printTable(topTable);
	#endif
	print_node_code($$);
};

definicoes_globais: definicao_global definicoes_globais {
        if ($1 != NULL) {
            if ($2 != NULL) {
                $$ = $1;
                adiciona_filho($$, $2);
				append_node_codes($$,$2);
            } else {
                $$ = $1;
            }
        } else {
            $$ = $2;
        };
		prt_dbg("definicoes_globais"); }
	| { $$ = NULL; prt_dbg("definicoes_globais vazia"); };

definicao_global: declaracao_variavel_externa { $$ = NULL; prt_dbg("definicao_global"); }
| definicao_de_funcao { $$ = $1; prt_dbg("definicao_global"); };

declaracao_variavel_externa: tipo especificacao_variaveis ',' { 
	addIdentifier(&tableStack, ($2->valor_lexico).label, $1, false, true, get_line_number());

	for (int i = 0; i < $2->num_filhos; i++) {
        if ($2->filhos[i]) {
			addIdentifier(&tableStack, (($2->filhos[i])->valor_lexico).label, $1, false, true, get_line_number());
        }
    }
};

especificacao_variaveis: TK_IDENTIFICADOR { $$ = cria_nodo($1); }
	| TK_IDENTIFICADOR ';' especificacao_variaveis { $$ = cria_nodo($1); adiciona_filho($$, $3); };

definicao_de_funcao: cabecalho_funcao corpo_funcao { 
	$$ = $1;
	adiciona_filho($$, $2);
	gen_func_declaration($1, $2, tableStack);
	prt_dbg("definicao_de_funcao");
};

cabecalho_funcao: abre_parametros_funcao argumentos_funcao ')' TK_OC_OR tipo '/' TK_IDENTIFICADOR {
	$$ = cria_nodo_v2($7, $5);
	addIdentifier(&tableStack, $7.label, $5, true, true, get_line_number());
	capture_params($$, $2);
	prt_dbg("cabecalho_funcao"); 
};

abre_parametros_funcao: '(' {
	createTableOnTop(&tableStack);
	prt_dbg("abre_parametros_funcao"); 
}

argumentos_funcao: lista_parametros { 
	$$ = $1;
	prt_dbg("argumentos_funcao"); 
} 
	| { $$ = NULL; prt_dbg("argumentos_funcao"); };

lista_parametros: parametro { $$ = $1; prt_dbg("lista_parametros"); }
	| parametro ';' lista_parametros { $$ = $1; adiciona_filho($1,$3); prt_dbg("lista_parametros"); };

parametro: tipo TK_IDENTIFICADOR { 
	$$ = cria_nodo_v2($2, $1);
	addIdentifier(&tableStack, $2.label, $1, false, false, get_line_number());
	HashTable* topTable = getTop(&tableStack);
	update_last_offset(topTable->curr_offset);
	prt_dbg("parametro"); 
};

tipo: TK_PR_INT { $$ = INT; prt_dbg("tipo (int)"); }
	| TK_PR_FLOAT { $$ = FLOAT ; prt_dbg("tipo (float)"); }
	| TK_PR_BOOL { $$ = BOOL ; prt_dbg("tipo (bool)"); };

corpo_funcao: bloco_instrucoes_funcao { $$ = $1; prt_dbg("corpo_funcao"); } ;

bloco_instrucoes_funcao: '{' sequencia_comandos fecha_escopo { 
	$$ = $2;
	prt_dbg("bloco_instrucoes"); 
} ;
	| '{' '}' { $$ = NULL; prt_dbg("bloco_instrucoes (empty)"); } ;

abre_escopo: '{' {
	createTableOnTop(&tableStack);
	prt_dbg("abre escopo");
}

fecha_escopo: '}' {
	HashTable* topTable = getTop(&tableStack);
	#if DEBUG_PARSER
	printTable(topTable);
	#endif
	update_last_offset(topTable->curr_offset);
	dropTop(tableStack);
	prt_dbg("fecha escopo");
}

bloco_instrucoes: abre_escopo sequencia_comandos fecha_escopo { 
	$$ = $2;
	prt_dbg("bloco_instrucoes"); 
} ;
	| '{' '}' { $$ = NULL; prt_dbg("bloco_instrucoes (empty)"); } ;

sequencia_comandos: instrucao_simples sequencia_comandos {
        if ($1 != NULL) {
            if ($2 != NULL) {
                $$ = $1;
                adiciona_filho($$, $2);
				append_node_codes($$, $2);
            } else {
                $$ = $1;
            }
        } else {
            $$ = $2;
        }
        prt_dbg("sequencia_comandos");
    }
	| instrucao_simples { 
		if ($1 != NULL) { 
			$$ = $1;
		}; 
		prt_dbg("sequencia_comandos"); 
	} ;

instrucao_simples: declaracao_variavel_interna ',' { $$ = NULL; prt_dbg("instrucao_simples (declaracao_variavel_interna)"); }
	| atribuicao ','  { 
		$$ = $1; 
		prt_dbg("instrucao_simples (atribuicao)"); 
	}
	| retorno_funcao ',' { $$ = $1; prt_dbg("instrucao_simples (retorno_funcao)"); }
	| estrutura_condicional  { $$ = $1; prt_dbg("instrucao_simples (estrutura_condicional)"); }
	| bloco_while { $$ = $1; prt_dbg("instrucao_simples (bloco_while)"); }
	| invocacao_funcao ',' { $$ = $1; prt_dbg("instrucao_simples (invocacao_funcao)"); };
	| bloco_instrucoes { $$ = $1; prt_dbg("instrucao_simples (bloco_instrucoes)"); };
	| ',' { $$ = NULL; prt_dbg("instrucao_simples (apenas vírgula)"); };

atribuicao: variavel '=' expressao { 
	$$ = cria_nodo($2);
	
	Nodo* var = makeNodeFromIdentifier($1);
	adiciona_filho($$, var);

	adiciona_filho($$, $3);
	updateIdentifier(tableStack, $1->name, ($3->valor_lexico).valor, get_line_number());
	gen_assignment($$, var, $1->isGlobal, $3);
	prt_dbg("atribuicao"); 
};

invocacao_funcao: TK_IDENTIFICADOR '(' lista_argumentos_funcao ')' {
		char* label = checkNatureAndGetLabel(tableStack, $1.label, true, get_line_number());
		$$ = cria_nodo(cria_call($1)); 
		adiciona_filho($$, $3);
		gen_call_func($$, $3, label, tableStack);
		prt_dbg("invocacao_funcao"); 
	} 
	| TK_IDENTIFICADOR '(' ')' {
		char* label = checkNatureAndGetLabel(tableStack, $1.label, true, get_line_number());
		$$ = cria_nodo(cria_call($1)); 
		gen_call_func($$, NULL, label, tableStack);
		prt_dbg("invocacao_funcao (no args)"); 
	};

declaracao_variavel_interna: tipo especificacao_variaveis_internas {
	addIdentifier(&tableStack, ($2->valor_lexico).label, $1, false, false, get_line_number());

	for (int i = 0; i < $2->num_filhos; i++) {
        if ($2->filhos[i]) {
			addIdentifier(&tableStack, (($2->filhos[i])->valor_lexico).label, $1, false, false, get_line_number());
        }
    }

	prt_dbg("declaracao_variavel_interna"); 
};

especificacao_variaveis_internas: TK_IDENTIFICADOR ';' especificacao_variaveis_internas { 
	$$ = cria_nodo($1);
	adiciona_filho($$, $3);
	prt_dbg("especificacao_variaveis_internas"); 
}
	| TK_IDENTIFICADOR { $$ = cria_nodo($1); prt_dbg("especificacao_variaveis_internas"); };

retorno_funcao: TK_PR_RETURN expressao { 
	$$ = cria_nodo_v2(cria_valor_lexico("return"), $2->tipo); 
	adiciona_filho($$, $2); 
	gen_return($$, $2);
	prt_dbg("retorno_funcao");
};

estrutura_condicional: TK_PR_IF '(' expressao ')' bloco_instrucoes { 
	$$ = cria_nodo_v2(cria_valor_lexico("if"), $5->tipo); 
	adiciona_filho($$, $3); 
	adiciona_filho($$, $5); 
	prt_dbg("estrutura_condicional"); 
	gen_if($$, $3, $5, NULL);
}
	| TK_PR_IF '(' expressao ')' bloco_instrucoes TK_PR_ELSE bloco_instrucoes {
		$$ = cria_nodo_v2(cria_valor_lexico("if"), $5->tipo);
		adiciona_filho($$, $3); 
		adiciona_filho($$, $5);
		adiciona_filho($$, $7);
		prt_dbg("estrutura_condicional");
		gen_if($$, $3, $5, $7);
	};

bloco_while: TK_PR_WHILE '(' expressao ')' bloco_instrucoes { 
	$$ = cria_nodo_v2(cria_valor_lexico("while"), $5->tipo); 
	adiciona_filho($$, $3); 
	adiciona_filho($$, $5);
	gen_while($$, $3, $5);
	prt_dbg("bloco_while"); 
} ;

argumento_funcao: expressao { $$ = $1; prt_dbg("argumento_funcao"); };

lista_argumentos_funcao: argumento_funcao ';' lista_argumentos_funcao { $$ = $1; adiciona_filho($$, $3); prt_dbg("lista_argumentos_funcao"); }
	| argumento_funcao { $$ = $1; prt_dbg("lista_argumentos_funcao"); };

argumentos_funcao: lista_argumentos_funcao { $$ = $1; prt_dbg("argumentos_funcao"); };

expressao: expressao TK_OC_OR expressao2 { 
	$$ = cria_nodo_v2($2, type_infer($1, $3)); 
	adiciona_filho($$, $1); 
	adiciona_filho($$, $3);
	gen_bin_expr($$, $1, $3);
	prt_dbg("expressao OR"); 
}
	| expressao2 { $$ = $1; prt_dbg("expressao"); };

expressao2: expressao2 TK_OC_AND expressao3 { 
	$$ = cria_nodo_v2($2, type_infer($1, $3)); 
	adiciona_filho($$, $1); 
	adiciona_filho($$, $3);
	gen_bin_expr($$, $1, $3);
	prt_dbg("expressao2 (AND)"); 
}
	| expressao3 { $$ = $1; prt_dbg("expressao2"); };

expressao3: expressao3 operador_bin_prec5 expressao4 { 
		$$ = cria_nodo_v2($2, type_infer($1, $3)); 
		adiciona_filho($$, $1); 
		adiciona_filho($$, $3); 
		gen_bin_expr($$, $1, $3);
		prt_dbg("expressao3"); 
	}
	| expressao4 { $$ = $1; prt_dbg("expressao3"); };

expressao4: expressao4 operador_bin_prec4 expressao5 { 
	$$ = cria_nodo_v2($2, type_infer($1, $3)); 
	adiciona_filho($$, $1); 
	adiciona_filho($$, $3); 
	gen_bin_expr($$, $1, $3);
	prt_dbg("expressao4"); 
}
	| expressao5 { $$ = $1; prt_dbg("expressao4"); };

expressao5: expressao5 operador_bin_prec3 expressao6 { 
	$$ = cria_nodo_v2($2, type_infer($1, $3)); 
	adiciona_filho($$, $1); 
	adiciona_filho($$, $3); 
	gen_bin_expr($$, $1, $3);
	prt_dbg("expressao5"); 
}
	| expressao6 { $$ = $1; prt_dbg("expressao5"); };

expressao6: expressao6 operador_bin_prec2 expressao7 { 
	$$ = cria_nodo_v2($2, type_infer($1, $3)); 
	adiciona_filho($$, $1); 
	adiciona_filho($$, $3); 
	gen_bin_expr($$, $1, $3);
	prt_dbg("expressao6"); 
}
	| expressao7 { $$ = $1; prt_dbg("expressao6"); };

expressao7: '-' expressao8 { 
	$$ = cria_nodo_v2($1, $2->tipo); 
	adiciona_filho($$, $2); 
	gen_invert_signal($$, $2);
	prt_dbg("expressao7");
}
	| '!' expressao8 { 
		$$ = cria_nodo_v2($1, $2->tipo); 
		adiciona_filho($$, $2); 
		gen_logic_invert($$, $2);
		prt_dbg("expressao7"); 
	}
	| expressao8 { $$ = $1; prt_dbg("expressao7"); };

expressao8: expressao_terminal { $$ = $1; prt_dbg("expressao8"); }
	| expressao9 { $$ = $1; prt_dbg("expressao8"); };

expressao9: '(' expressao ')'  { $$ = $2; prt_dbg("expressao9"); };

variavel: TK_IDENTIFICADOR {
	$$ = findIdentifier(tableStack, $1.label, false, get_line_number()); 
	prt_dbg("variavel"); 
}

expressao_terminal: variavel {
	Nodo* var = makeNodeFromIdentifier($1);
	gen_load_var(var, $1->isGlobal);
	$$ = var;
	prt_dbg("expressao_terminal (variavel)");
}
	| invocacao_funcao { 
		$$ = $1;
		prt_dbg("expressao_terminal (invocacao_funcao)"); 
	}
	| literal  { $$ = $1; prt_dbg("expressao_terminal (literal)"); }
    ;

literal:  TK_LIT_TRUE { $$ = cria_nodo_v2($1, BOOL); gen_load_literal($$); prt_dbg("literal (TK_LIT_TRUE)"); }
	| TK_LIT_FALSE { $$ = cria_nodo_v2($1, BOOL); gen_load_literal($$); prt_dbg("literal (TK_LIT_FALSE)"); }
	| TK_LIT_FLOAT  { $$ = cria_nodo_v2($1, FLOAT); prt_dbg("literal (TK_LIT_FLOAT)"); }
	| TK_LIT_INT { $$ = cria_nodo_v2($1, INT); gen_load_literal($$); prt_dbg("literal (TK_LIT_INT)"); };

operador_bin_prec2: '*' { $$ = $1; prt_dbg("operador_bin_prec2"); }
	| '/' { $$ = $1; prt_dbg("operador_bin_prec2"); }
	| '%' { $$ = $1; prt_dbg("operador_bin_prec2"); };

operador_bin_prec3: '+' { $$ = $1; prt_dbg("operador_bin_prec3"); }
	| '-' { $$ = $1; prt_dbg("operador_bin_prec3"); };

operador_bin_prec4: '<' { $$ = $1; prt_dbg("operador_bin_prec4"); }
	| '>' { $$ = $1; prt_dbg("operador_bin_prec4"); }
	| TK_OC_LE { $$ = $1; prt_dbg("operador_bin_prec4"); }
	| TK_OC_GE { $$ = $1; prt_dbg("operador_bin_prec4"); };

operador_bin_prec5: TK_OC_EQ { $$ = $1; prt_dbg("operador_bin_prec5"); }
	| TK_OC_NE  { $$ = $1; prt_dbg("operador_bin_prec5"); };

%%

void yyerror (char const *s) {
   printf("Error at line %d: %s\n", get_line_number(), s);
}