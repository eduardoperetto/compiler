#include <stdio.h>
// #define DEBUG_MAIN 
extern int yyparse(void);
extern int yylex_destroy(void);
void *arvore = NULL;
void exporta (void *arvore);
int main (int argc, char **argv)
{
  int ret = yyparse(); 
  exporta (arvore);
  yylex_destroy();
  #ifdef DEBUG_MAIN
  printf("\nProgram returns %d\n", ret);
  #endif
  return ret;
}
