#include <stdio.h>
#include <stdlib.h>

int main() {

   void *pointer = malloc(16);
   int *i;

   *((int *)pointer) = 5;
   *((void **)(pointer+4)) = pointer;
   i = *((void **)(pointer+4));
   *i = *i - 1;
   *((int *)(pointer+12)) = 10;

   printf("%d, %d, %d\n", *((int *)pointer), *i, *((int *)(pointer+12))); 

   return 1;
}
