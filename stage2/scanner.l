%{
    #include "parser.tab.h"
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
""if"" { return TK_PR_IF; }
""else"" { return TK_PR_ELSE; }
""while"" { return TK_PR_WHILE; }
""return"" { return TK_PR_RETURN; }

[\-\!\*\/\%\+\-\<\>\{\}\(\)\=,\;]  {return yytext[0];}

\/\/.*
\/\* { BEGIN(COMMENT); }

<COMMENT>\*\/                   { BEGIN(INITIAL); }
<COMMENT>.
<COMMENT>"\n"                   

"<="    {return TK_OC_LE;}
">="    {return TK_OC_GE;}
"=="    {return TK_OC_EQ;}
"!="    {return TK_OC_NE;}
"&" {return TK_OC_AND;}
"|" {return TK_OC_OR;}


[0-9]+ {return TK_LIT_INT;}
[0-9]*\.[0-9]+ {return TK_LIT_FLOAT;}
true {return TK_LIT_TRUE;}
false {return TK_LIT_FALSE;}

[_a-z][_a-z0-9]* {return TK_IDENTIFICADOR;}


. { return TK_ERRO; }

%%


int get_line_number(void) {
    return yylineno;
}
