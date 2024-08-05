#include <stdio.h>
extern int yyparse(void);
extern int yylex_destroy(void);
void *tree = NULL;
void exporta (void *tree);
int main (int argc, char **argv)
{
  int ret = yyparse(); 
  exporta (tree);
  yylex_destroy();
  return ret;
}
