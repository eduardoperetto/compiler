#include <stdio.h>
#define DEBUG_MAIN 0
extern int yyparse(void);
extern int yylex_destroy(void);
void *tree = NULL;
void exporta (void *tree);
int main (int argc, char **argv)
{
  int ret = yyparse(); 
  exporta (tree);
  yylex_destroy();
  #if DEBUG_MAIN
  printf("Program returns %d\n", ret);
  #endif
  return ret;
}
