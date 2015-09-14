#include <stdio.h>
#include <stdlib.h>

int main() {
   void *pg = malloc(4);

   pg = pg - 32;

   printf("%d, %d\n", *((int *)pg), *((int *)(pg + 4)));

   return 1;
}
