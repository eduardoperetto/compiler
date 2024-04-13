%{
int yylex(void);
void yyerror (char const *mensagem);
int get_line_number(void);
#define YYDEBUG 1
%}

%define parse.error verbose

%token TK_PR_INT
%token TK_PR_FLOAT
%token TK_PR_BOOL
%token TK_PR_IF
%token TK_PR_ELSE
%token TK_PR_WHILE
%token TK_PR_RETURN
%token TK_OC_LE
%token TK_OC_GE
%token TK_OC_EQ
%token TK_OC_NE
%token TK_OC_AND
%token TK_OC_OR
%token TK_IDENTIFICADOR
%token TK_LIT_INT
%token TK_LIT_FLOAT
%token TK_LIT_FALSE
%token TK_LIT_TRUE
%token TK_ERRO

%%

programa: definicoes_globais;

definicoes_globais: definicao_global definicoes_globais | ;

definicao_global: declaracao_variavel_externa | definicao_de_funcao;

declaracao_variavel_externa: tipo especificacao_variaveis ';';

especificacao_variaveis: TK_IDENTIFICADOR | TK_IDENTIFICADOR ',' especificacao_variaveis;

definicao_de_funcao: estrutura_funcao corpo_funcao;

estrutura_funcao: tipo TK_IDENTIFICADOR '(' argumentos_funcao ')';

argumentos_funcao: lista_parametros | ;

lista_parametros: parametro | parametro ',' lista_parametros;

parametro: tipo TK_IDENTIFICADOR;

tipo: TK_PR_INT | TK_PR_FLOAT | TK_PR_BOOL;

corpo_funcao: bloco_instrucoes;

bloco_instrucoes: '{' sequencia_comandos '}' ;

sequencia_comandos: instrucao_simples ';' sequencia_comandos | instrucao_simples ';' ;

instrucao_simples: declaracao_variavel_interna 
                 | atribuicao
                 | retorno_funcao
                 | estrutura_condicional 
                 | bloco_while
                 | invocacao_funcao
                 ;

atribuicao: TK_IDENTIFICADOR '=' expressao;

invocacao_funcao: TK_IDENTIFICADOR '(' lista_argumentos_funcao ')';

declaracao_variavel_interna: tipo especificacao_variaveis_internas;

especificacao_variaveis_internas: nome_variavel_inicial ',' especificacao_variaveis_internas | nome_variavel_inicial;
                               
nome_variavel_inicial: TK_IDENTIFICADOR TK_OC_EQ expressao
                      | TK_IDENTIFICADOR
                      ;

retorno_funcao: TK_PR_RETURN expressao;

estrutura_condicional: TK_PR_IF '(' expressao ')' bloco_instrucoes
                      | TK_PR_IF '(' expressao ')' bloco_instrucoes TK_PR_ELSE bloco_instrucoes
                      ;

bloco_while: TK_PR_WHILE '(' expressao ')' bloco_instrucoes ;

argumento_funcao: expressao;

lista_argumentos_funcao: argumento_funcao ',' lista_argumentos_funcao | argumento_funcao;

argumentos_funcao: lista_argumentos_funcao ;

expressao: expressao_prec7;

expressao_prec7: expressao_prec6 | expressao_bin_or;
expressao_prec6: expressao_prec5 | expressao_bin_and;
expressao_prec5: expressao_prec4 | expressao_bin_eq;
expressao_prec4: expressao_prec3 | expressao_bin_compare;
expressao_prec3: expressao_prec2 | expressao_bin_aritm;
expressao_prec2: expressao_prec1 | expressao_bin_mult;
expressao_prec1: expressao_prec0 | expressao_unaria;
expressao_prec0: '(' expressao ')' | expressao_terminal;
expressao_terminal: TK_IDENTIFICADOR
	| invocacao_funcao
	| literal 
    ;  

literal:  TK_LIT_TRUE
             | TK_LIT_FALSE
             | TK_LIT_FLOAT 
             | TK_LIT_INT
             ;

expressao_unaria: '-' expressao_terminal | '!' expressao_terminal;

expressao_bin_mult: expressao_terminal operador_bin_prec2 expressao_terminal;
operador_bin_prec2: '*' | '/' | '%';

expressao_bin_aritm: expressao_terminal operador_bin_prec3 expressao_terminal;
operador_bin_prec3: '+' | '-';

expressao_bin_compare: expressao_terminal operador_bin_prec4 expressao_terminal;
operador_bin_prec4: '<' | '>' | TK_OC_LE | TK_OC_GE;

expressao_bin_eq: expressao_terminal operador_bin_prec5 expressao_terminal;
operador_bin_prec5: TK_OC_EQ | TK_OC_NE;

expressao_bin_and: expressao_terminal TK_OC_AND expressao_terminal;

expressao_bin_or: expressao_terminal TK_OC_OR expressao_terminal;

%%

void yyerror (char const *s) {
   printf("Error at line %d: %s\n", get_line_number(), s);
}