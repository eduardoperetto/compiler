%{
/**
* Projeto Compiladores INF01147 2024/1
* Alunos: 
* Eduardo Raupp Peretto - 00313439
* João Pedro Alves Liz - 00333681
*
**/

    #include "tree.h"
    #include "parser.tab.h"
    int get_line_number(void);
%}

%OPTION NOYYWRAP
%OPTION YYLINENO
%x COMMENT

%%

\n
[ \t]

""int"" { return TK_PR_INT; }
""float"" { return TK_PR_FLOAT; }
""bool"" { return TK_PR_BOOL; }

""if"" { yylval.valor_lexico = atribui_yylval(yytext, NONE, get_line_number()); return TK_PR_IF; }
""else"" { yylval.valor_lexico = atribui_yylval(yytext, NONE, get_line_number()); return TK_PR_ELSE; }
""while"" { yylval.valor_lexico = atribui_yylval(yytext, NONE, get_line_number()); return TK_PR_WHILE; }
""return"" { yylval.valor_lexico = atribui_yylval(yytext, NONE, get_line_number()); return TK_PR_RETURN; }

[\-\!\*\/\%\+\-\<\>\{\}\(\)\=,\;]  { yylval.valor_lexico = atribui_yylval(yytext, NONE, get_line_number()); return yytext[0];}

\/\/.*
\/\* { BEGIN(COMMENT); }

<COMMENT>\*\/                   { BEGIN(INITIAL); }
<COMMENT>.
<COMMENT>"\n"                   

"<="    { yylval.valor_lexico = atribui_yylval(yytext, OPERADOR_COMPOSTO, get_line_number()); return TK_OC_LE;}
">="    { yylval.valor_lexico = atribui_yylval(yytext, OPERADOR_COMPOSTO, get_line_number()); return TK_OC_GE;}
"=="    { yylval.valor_lexico = atribui_yylval(yytext, OPERADOR_COMPOSTO, get_line_number()); return TK_OC_EQ;}
"!="    { yylval.valor_lexico = atribui_yylval(yytext, OPERADOR_COMPOSTO, get_line_number()); return TK_OC_NE;}
"&" { yylval.valor_lexico = atribui_yylval(yytext, OPERADOR_COMPOSTO, get_line_number()); return TK_OC_AND;}
"|" { yylval.valor_lexico = atribui_yylval(yytext, OPERADOR_COMPOSTO, get_line_number()); return TK_OC_OR;}

[0-9]+ { yylval.valor_lexico = atribui_yylval(yytext, LIT_INTEIRO, get_line_number()); return TK_LIT_INT;}
[0-9]*\.[0-9]+ { yylval.valor_lexico = atribui_yylval(yytext, TK_LIT_FLOAT, get_line_number()); return TK_LIT_FLOAT;}
true { yylval.valor_lexico = atribui_yylval(yytext, LIT_BOOL, get_line_number()); return TK_LIT_TRUE;}
false { yylval.valor_lexico = atribui_yylval(yytext, LIT_BOOL, get_line_number()); return TK_LIT_FALSE;}

[_a-z][_a-z0-9]* { yylval.valor_lexico = atribui_yylval(yytext, IDENTIFICADOR, get_line_number()); return TK_IDENTIFICADOR;}

. { TK_ERRO; }

%%

int get_line_number(void) {
   return yylineno;
}
